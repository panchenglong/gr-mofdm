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

#ifndef INCLUDED_MOFDM_OFDM_DATA_SINK_IMPL_H
#define INCLUDED_MOFDM_OFDM_DATA_SINK_IMPL_H

#include <mofdm/ofdm_data_sink.h>
#include <cstdlib>
#include <cstdio>
#include <boost/crc.hpp>

namespace gr {
  namespace mofdm {

    class ofdm_data_sink_impl : public ofdm_data_sink
    {
     private:
      FILE *d_log;
      boost::crc_optimal<32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true> d_crc_impl;
      int d_npass;
      int d_nfail;

     public:
      ofdm_data_sink_impl(const std::string& lengthtagname);
      ~ofdm_data_sink_impl();

      // Where all the action really happens
      int work(int noutput_items,
        gr_vector_int &ninput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace mofdm
} // namespace gr

#endif /* INCLUDED_MOFDM_OFDM_DATA_SINK_IMPL_H */

