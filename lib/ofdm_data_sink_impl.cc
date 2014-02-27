/* -*- c++ -*- */
/* 
 * Copyright 2013 <+YOU OR YOUR COMPANY+>.
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
#include "ofdm_data_sink_impl.h"
#include <string>

namespace gr {
  namespace mofdm {

    ofdm_data_sink::sptr
    ofdm_data_sink::make(const std::string& lengthtagname)
    {
      return gnuradio::get_initial_sptr
        (new ofdm_data_sink_impl(lengthtagname));
    }

    /*
     * The private constructor
     */
    ofdm_data_sink_impl::ofdm_data_sink_impl(const std::string& lengthtagname)
      : gr::tagged_stream_block("ofdm_data_sink",
              gr::io_signature::make(1, 1, sizeof (char)),
              gr::io_signature::make(0, 0, 0),
              lengthtagname),
      d_npass(0), d_nfail(0)
    {
      d_log = fopen("ofdm_data_sink.dat", "w");
      set_tag_propagation_policy(TPP_DONT);
    }

    /*
     * Our virtual destructor.
     */
    ofdm_data_sink_impl::~ofdm_data_sink_impl()
    {
      fprintf(d_log, "\n\nsuccesfully received = %d\nfailed = %d\npacket error rate = %f\n", 
        d_npass, d_nfail, (float)d_nfail/(d_npass + d_nfail));

      fclose(d_log);
    }

    int
    ofdm_data_sink_impl::work(int noutput_items,
        gr_vector_int &ninput_items,
    	  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
      const unsigned char *in = (const unsigned char *) input_items[0];
      long packet_length = ninput_items[0];
      unsigned int crc;
      bool ok;

      d_crc_impl.reset();
      d_crc_impl.process_bytes(in, packet_length-4);
      crc = d_crc_impl();
      if (crc != *(unsigned int *)(in+packet_length-4)) { // CRC Failed
        d_nfail++;
        ok = false;
      } else {
        d_npass++;
        ok = true;
      }

      const char table [16+1] = "0123456789abcdef";
      char _pktno[2];
      _pktno[0] = in[1];
      _pktno[1] = in[0];
      unsigned short pktno = *(unsigned short*)_pktno;
      //in += 2;

      std::string payload;
      for (int i = 0; i < packet_length - 4; i++) {
        payload.append(1, table[(in[i] >> 4) & 0x0f]);
        payload.append(1, table[in[i] & 0x0f]);
      }
      //memcpy((void *) out, (const void *) in, packet_length-4);
      fprintf(d_log, "\npacket %u, ok = %s, payload (include pktno) = \n%s\n", pktno, ok ? "true" : "false", payload.c_str());

      return noutput_items;
    }

  } /* namespace mofdm */
} /* namespace gr */

