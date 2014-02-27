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


#ifndef INCLUDED_MOFDM_OFDM_SUBCHAN_SNR_VCVF_H
#define INCLUDED_MOFDM_OFDM_SUBCHAN_SNR_VCVF_H

#include <mofdm/api.h>
#include <gnuradio/tagged_stream_block.h>
#include <gnuradio/digital/constellation.h>

namespace gr {
  namespace mofdm {

    /*!
     * \brief <+description of block+>
     * \ingroup mofdm
     *
     */
    class MOFDM_API ofdm_subchan_snr_vcvf : virtual public tagged_stream_block
    {
     public:
      typedef boost::shared_ptr<ofdm_subchan_snr_vcvf> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of mofdm::ofdm_subchan_snr_vcvf.
       *
       * To avoid accidental use of raw pointers, mofdm::ofdm_subchan_snr_vcvf's
       * constructor is in a private implementation
       * class. mofdm::ofdm_subchan_snr_vcvf::make is the public interface for
       * creating new instances.
       */
      static sptr make(int fft_len, int cp_len,
          //gr::digital::constellation_sptr constellation,
          const std::string& len_tag_key = "frame_len",    
          const std::vector<std::vector<int> > &occupied_carriers = std::vector<std::vector<int> >(),
          const std::vector<std::vector<int> > &pilot_carriers = std::vector<std::vector<int> >(),
          const std::vector<std::vector<gr_complex> > &pilot_symbols = std::vector<std::vector<gr_complex> >(),
          int symbols_skipped=0,
          bool input_is_shifted=true,
          bool debug=false
          );
    };

  } // namespace mofdm
} // namespace gr

#endif /* INCLUDED_MOFDM_OFDM_SUBCHAN_SNR_VCVF_H */

