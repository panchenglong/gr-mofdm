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
#include "rx_sample_metadata_impl.h"
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <complex>

namespace gr {
  namespace mofdm {

    rx_sample_metadata::sptr
    rx_sample_metadata::make()
    {
      return gnuradio::get_initial_sptr
        (new rx_sample_metadata_impl());
    }

    /*
     * The private constructor
     */
    rx_sample_metadata_impl::rx_sample_metadata_impl()
      : gr::sync_block("rx_sample_metadata",
              gr::io_signature::make(1, 1, sizeof(std::complex<float>)),
              gr::io_signature::make(0, 0, 0))
    {
      d_dump.open("rx_sample_metadata.dat");
    }

    /*
     * Our virtual destructor.
     */
    rx_sample_metadata_impl::~rx_sample_metadata_impl()
    {
      d_dump.close();
    }

    int
    rx_sample_metadata_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        //grab all "rx time" tags in this work call
        int ninput_items = noutput_items;
        const uint64_t samp0_count = this->nitems_read(0);
        std::vector<gr::tag_t> rx_time_tags;
        get_tags_in_range(rx_time_tags, 0, samp0_count, samp0_count + ninput_items, pmt::string_to_symbol("rx_time"));

        //print all tags
        BOOST_FOREACH(const gr::tag_t &rx_time_tag, rx_time_tags){
            const uint64_t offset = rx_time_tag.offset;
            const pmt::pmt_t &value = rx_time_tag.value;

            d_dump << boost::format("Full seconds %u, Frac seconds %f, abs sample offset %u")
                % pmt::to_uint64(pmt::tuple_ref(value, 0))
                % pmt::to_double(pmt::tuple_ref(value, 1))
                % offset
            << std::endl;
        }

        return ninput_items;
    }

  } /* namespace mofdm */
} /* namespace gr */

