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

#ifndef INCLUDED_MOFDM_OFDM_MAC_IMPL_H
#define INCLUDED_MOFDM_OFDM_MAC_IMPL_H

#include <mofdm/ofdm_mac.h>
#include <stdint.h>
#include <queue>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace gr {
  namespace mofdm {

    struct mac_header
    {
      uint16_t frame_control; // currently only used to distinct between data frame and ACK frame
      uint8_t src_addr[6];
      uint8_t dst_addr[6];
      uint16_t seq_nr;
    };

    class ofdm_mac_impl : public ofdm_mac
    {
      enum state_t { READY_TO_SEND, WAIT_FOR_ACK };

     private:
      uint8_t d_mac_addr[6];
      uint16_t d_seq_nr;
      uint16_t d_expected_seq_nr;
      std::queue<pmt::pmt_t> d_queue;
      state_t d_state;
      bool d_debug;
      boost::posix_time::ptime d_tx_time;
      long d_timeout; // in milliseconds
      int d_retries;
      int d_max_attempts;
      pmt::pmt_t d_resend_msg;
//    boost::shared_ptr<boost::thread> d_thread;

      // statistics
      int d_duplicate_data; // retransmition data frame receiver has received
      int d_duplicate_ack; // ACK frames corresponding to the last frame the sender has received

      void phy_rx(pmt::pmt_t msg);
      void encap_rx(pmt::pmt_t msg);
      void generate_mac_data_frame(const uint8_t *msdu, int msdu_size, const uint8_t *dst, uint8_t **psdu, int *psdu_size);
      bool accept_frame(const uint8_t *dst_addr);
      inline void send_packet(pmt::pmt_t pkt);
      void send_ack(const uint8_t *dst_addr, uint16_t seq_nr);

      void print_msg(const uint8_t *msg, size_t len);

     public:
      ofdm_mac_impl(const std::string &mac_addr, long timeout, int max_attempts, bool debug);
      ~ofdm_mac_impl();

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace mofdm
} // namespace gr

#endif /* INCLUDED_MOFDM_OFDM_MAC_IMPL_H */

