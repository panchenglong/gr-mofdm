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


#ifndef INCLUDED_MOFDM_OFDM_CYCLIC_PREFIXER_H
#define INCLUDED_MOFDM_OFDM_CYCLIC_PREFIXER_H

#include <mofdm/api.h>
#include <gnuradio/tagged_stream_block.h>

namespace gr {
  namespace mofdm {

    /*!
     * \brief <+description of block+>
     * \ingroup mofdm
     *
     */
    class MOFDM_API ofdm_cyclic_prefixer : virtual public gr::tagged_stream_block
    {
     public:
      typedef boost::shared_ptr<ofdm_cyclic_prefixer> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of mofdm::ofdm_cyclic_prefixer.
       *
       * To avoid accidental use of raw pointers, mofdm::ofdm_cyclic_prefixer's
       * constructor is in a private implementation
       * class. mofdm::ofdm_cyclic_prefixer::make is the public interface for
       * creating new instances.
       */
      static sptr make(size_t input_size,
        size_t output_size,
        int rolloff_len=0,
        const std::string &len_tag_key="");
    };

  } // namespace mofdm
} // namespace gr

#endif /* INCLUDED_MOFDM_OFDM_CYCLIC_PREFIXER_H */

