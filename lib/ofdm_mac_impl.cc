/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "ofdm_mac_impl.h"
#include <cassert>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <sstream>

namespace gr
{
namespace mofdm
{

ofdm_mac::sptr
ofdm_mac::make(const std::string& mac_addr, long timeout, int max_attempts, bool debug)
{
    return gnuradio::get_initial_sptr
           (new ofdm_mac_impl(mac_addr, timeout, max_attempts, debug));
}

/*
 * The private constructor
 */
ofdm_mac_impl::ofdm_mac_impl(const std::string& mac_addr, long timeout, int max_attempts, bool debug)
   : gr::block("ofdm_mac",
               gr::io_signature::make(1, 1, sizeof(unsigned char)),
               gr::io_signature::make(0, 0, 0)),
     d_seq_nr(0),
     d_expected_seq_nr(0),
     d_state(READY_TO_SEND),
     d_retries(0),
     d_timeout(timeout),
     d_max_attempts(max_attempts),
     d_resend_msg(pmt::PMT_NIL),
     d_duplicate_data(0),
     d_duplicate_ack(0),
     d_debug(debug)
{
    std::stringstream ss(mac_addr);
    char trash; // used as placeholder of the ":", this variable itself is meaningless

    int foo;
    for (int i = 0; i < 5; i++) {
        ss >> std::hex >> foo >> trash;
        d_mac_addr[i] = static_cast<uint8_t>(foo);
    }
    ss >> std::hex >> foo;
    d_mac_addr[5] = static_cast<uint8_t>(foo);

    message_port_register_out(pmt::mp("to_phy"));
    message_port_register_out(pmt::mp("to_encap"));

    message_port_register_in(pmt::mp("from_phy"));
    set_msg_handler(pmt::mp("from_phy"), boost::bind(&ofdm_mac_impl::phy_rx, this, _1));

    message_port_register_in(pmt::mp("from_encap"));
    set_msg_handler(pmt::mp("from_encap"), boost::bind(&ofdm_mac_impl::encap_rx, this, _1));

    message_port_register_out(pmt::mp("monitor_out"));
}

ofdm_mac_impl::~ofdm_mac_impl()
{
}

int ofdm_mac_impl::general_work(int noutput_items,
                                gr_vector_int& ninput_items,
                                gr_vector_const_void_star& input_items,
                                gr_vector_void_star& output_items)
{
    unsigned char *in = (unsigned char *)input_items[0];
    unsigned char *end = in + ninput_items[0];

    bool busy = false;
    while (in < end) {
        if (*in) {
            busy = true;
            break;
        }
        in++;
    }

    bool empty;
    switch (d_state) {

    case READY_TO_SEND:
        empty = d_queue.empty();

        if (!empty && !busy) {
            // Send new msg with seq no = d_seq_nr

            pmt::pmt_t msg = d_queue.front();
            d_queue.pop();

            size_t       msg_len;
            const uint8_t   *msdu;

            if (pmt::is_eof_object(msg)) return -1; // Done

            if (!pmt::is_pair(msg)) throw std::runtime_error("received a malformed pdu message, it should be a pmt pair");

            pmt::pmt_t meta = pmt::car(msg);
            if (pmt::eq(meta, pmt::PMT_NIL) || !pmt::is_dict(meta)) throw std::runtime_error("meta info such dst addr should exist");
            size_t len;
            const uint8_t *dst_addr = pmt::u8vector_elements(pmt::dict_ref(meta, pmt::mp("dst_addr"), pmt::PMT_NIL), len);
            assert(len == 6);

            pmt::pmt_t data = pmt::cdr(msg);
            msdu = pmt::u8vector_elements(data, len);
            msg_len = len;

            if (d_debug) {
                std::cerr << "send msg:\n";
                print_msg(msdu, len);
            }

            // make MAC frame
            int    psdu_length;
            uint8_t *psdu;

            generate_mac_data_frame(msdu, msg_len, dst_addr, &psdu, &psdu_length);

            pmt::pmt_t mac = pmt::init_u8vector(psdu_length, psdu);
            d_resend_msg = pmt::cons(pmt::PMT_NIL, mac);
            //free(dst_addr);
            free(psdu);
            send_packet(d_resend_msg);
            d_tx_time = boost::posix_time::microsec_clock::local_time();
            d_state = WAIT_FOR_ACK;
            d_retries = 0;
        }
        break;

    case WAIT_FOR_ACK:
        if (boost::posix_time::microsec_clock::local_time() - d_tx_time >
               boost::posix_time::milliseconds(d_timeout)) {
            if (d_retries == d_max_attempts) {
                std::cerr << "Retry times has reached max!! Give up this packet!" << std::endl;
                d_retries = 0;
                d_resend_msg = pmt::PMT_NIL; // TODO! notify the higher layer that the chances has tried out.
                d_state = READY_TO_SEND;
            } else {
                if (!busy) {
                    send_packet(d_resend_msg); // retransmit
                    d_tx_time = boost::posix_time::microsec_clock::local_time();
                    d_retries++;
                }
            }
        }
        break;

    default:
        throw std::runtime_error("unexpected state");
        break;
    }

    consume_each(ninput_items[0]);

    return 0;
} /* general_work() */

void ofdm_mac_impl::phy_rx(pmt::pmt_t msg)
{
    if (!pmt::is_pair(msg)) throw std::runtime_error("received a malformed pdu message, it should be a pmt pair");
    pmt::pmt_t data = pmt::cdr(msg);

    if (!pmt::is_u8vector(data)) throw std::runtime_error("data is not of type u8vector");

    const size_t header_size = sizeof(mac_header);

    size_t psdu_length;
    const uint8_t *psdu = u8vector_elements(data, psdu_length);

    assert(psdu_length >= header_size);

    const mac_header *header = reinterpret_cast<const mac_header *>(psdu);

    if (!accept_frame(header->dst_addr)) {
        // Frame not destined to me;
        return;
    }

    if (header->frame_control == 0x0000) { // user data frame, then send ack
        send_ack(header->src_addr, header->seq_nr); // need to backoff due to the busy channel??

        // Send ack, seq no = header->seq_nr
        if (header->seq_nr == d_expected_seq_nr) { // this is the first time receiver receive this frame
            d_expected_seq_nr++; // wait for the next msg

            pmt::pmt_t meta = pmt::make_dict();
            meta = pmt::dict_add(meta, pmt::mp("seq_nr"), pmt::mp(header->seq_nr));
            pmt::pmt_t user_data = pmt::init_u8vector(psdu_length - header_size, psdu + header_size);
            message_port_pub(pmt::mp("to_encap"), pmt::cons(meta, user_data)); // send to encap

            pmt::pmt_t mon_info = pmt::make_dict();
            mon_info = pmt::dict_add(mon_info, pmt::mp("duplicate_data"), pmt::mp(d_duplicate_data));
            message_port_pub(pmt::mp("monitor_out"), mon_info);
            d_duplicate_data = 0;

        } else { // this is the retransmitting frame
            // Throw away retransmition frame with seq no = header->seq_nr,
            // But we expect next frame with seq no = d_expected_seq_nr
            d_duplicate_data++;
        }

    } else if (header->frame_control == 0x0001) { // ack frame
        if (header->seq_nr == d_seq_nr) {
            // Received ack, seq no = header->seq_nr
            d_seq_nr++;
            d_resend_msg = pmt::PMT_NIL;
            d_state = READY_TO_SEND; // acked, now we can send next msg in the queue

            pmt::pmt_t mon_info = pmt::make_dict();
            mon_info = pmt::dict_add(mon_info, pmt::mp("seq_nr"), pmt::mp(header->seq_nr));
            mon_info = pmt::dict_add(mon_info, pmt::mp("retransmit_times"), pmt::mp(d_retries));
            mon_info = pmt::dict_add(mon_info, pmt::mp("duplicate_ack"), pmt::mp(d_duplicate_ack));
            message_port_pub(pmt::mp("monitor_out"), mon_info);
            d_duplicate_ack = 0;
        } else {
            d_duplicate_ack++;
        }

    } else {
        throw std::runtime_error("unexpected frame control value");
    }

    //free(psdu);
}

void ofdm_mac_impl::encap_rx(pmt::pmt_t msg)
{
    d_queue.push(msg);
}

void ofdm_mac_impl::generate_mac_data_frame(const uint8_t *msdu, int msdu_size,
                                            const uint8_t *dst,
                                            uint8_t **psdu, int *psdu_size)
{
    // mac header
    mac_header header;
    header.frame_control = 0x0000; // data frame, 0x0001 to be ack frame
                                   // NOTE!! HERE ONLY ONE BIT IS USED, SO I DON'T CARE
                                   // THE BYTE ORDER. ONCE MORE BITS ARE USED, WE MUST TAKE CARE
                                   // OF BIG ENDIAN AND LITTLE ENDIAN.
    std::memcpy(&header.src_addr, d_mac_addr, 6);
    std::memcpy(&header.dst_addr, dst, 6);
    header.seq_nr = d_seq_nr;

    int header_size = sizeof(mac_header);
    *psdu_size = header_size + msdu_size;
    *psdu = (uint8_t *)calloc(*psdu_size, sizeof(uint8_t));

    //copy mac header into psdu
    std::memcpy(*psdu, &header, header_size);
    //copy msdu into psdu
    std::memcpy(*psdu + header_size, msdu, msdu_size);
}

bool ofdm_mac_impl::accept_frame(const uint8_t *dst_addr)
{
    bool broadcast = true;
    for (int i = 0; i < 6; ++i) {
        if (dst_addr[i] != 0xff) {
            broadcast = false;
            break;
        }
    }
    if (broadcast) return true;

    for (int i = 0; i < 6; ++i) {
        if (d_mac_addr[i] != dst_addr[i]) {
            return false;
        }
    }
    return true;
}

inline void ofdm_mac_impl::send_packet(pmt::pmt_t pkt)
{
    message_port_pub(pmt::mp("to_phy"), pkt);
}

void ofdm_mac_impl::send_ack(const uint8_t *dst_addr, uint16_t seq_nr)
{
    mac_header header;
    header.frame_control = 0x0001; // 0x0001 to be ack frame
                                   // NOTE!! HERE ONLY ONE BIT IS USED, SO I DON'T CARE
                                   // THE BYTE ORDER. ONCE MORE BITS ARE USED, WE MUST TAKE CARE
                                   // OF BIG ENDIAN AND LITTLE ENDIAN.
    std::memcpy(&header.src_addr, d_mac_addr, 6);
    std::memcpy(&header.dst_addr, dst_addr, 6);
    header.seq_nr = seq_nr;

    pmt::pmt_t data = pmt::init_u8vector(sizeof(mac_header), reinterpret_cast<const uint8_t *>(&header));
    send_packet(pmt::cons(pmt::PMT_NIL, data));
}

void ofdm_mac_impl::print_msg(const uint8_t *msg, size_t len)
{
    std::cerr << std::hex;
    for (int i = 0; i < len; i++) std::cerr << static_cast<int>(msg[i]) << " ";
    std::cerr << "\n";
    std::cerr << std::dec;
}


} /* namespace mofdm */
} /* namespace gr */

