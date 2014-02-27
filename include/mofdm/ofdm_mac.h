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


#ifndef INCLUDED_MOFDM_OFDM_MAC_H
#define INCLUDED_MOFDM_OFDM_MAC_H

#include <mofdm/api.h>
#include <gnuradio/block.h>
#include <string>

namespace gr {
  namespace mofdm {

    /*!
     * \brief <+description of block+>
     * \ingroup mofdm
     *
     */
    class MOFDM_API ofdm_mac : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<ofdm_mac> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of mofdm::ofdm_mac.
       *  timeout in milliseconds 
       */
      static sptr make(const std::string &mac_addr, long timeout=500, int max_attempts=20, bool d_debug=false);
    };

  } // namespace mofdm
} // namespace gr

#endif /* INCLUDED_MOFDM_OFDM_MAC_H */

