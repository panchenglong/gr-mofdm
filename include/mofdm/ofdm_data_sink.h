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


#ifndef INCLUDED_MOFDM_OFDM_DATA_SINK_H
#define INCLUDED_MOFDM_OFDM_DATA_SINK_H

#include <mofdm/api.h>
#include <gnuradio/tagged_stream_block.h>

namespace gr {
  namespace mofdm {

    /*!
     * \brief <+description of block+>
     * \ingroup mofdm
     *
     */
    class MOFDM_API ofdm_data_sink : virtual public tagged_stream_block
    {
     public:
      typedef boost::shared_ptr<ofdm_data_sink> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of mofdm::ofdm_data_sink.
       *
       * To avoid accidental use of raw pointers, mofdm::ofdm_data_sink's
       * constructor is in a private implementation
       * class. mofdm::ofdm_data_sink::make is the public interface for
       * creating new instances.
       */
      static sptr make(const std::string& lengthtagname="packet_len");
    };

  } // namespace mofdm
} // namespace gr

#endif /* INCLUDED_MOFDM_OFDM_DATA_SINK_H */

