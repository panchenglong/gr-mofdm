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

#ifndef INCLUDED_MOFDM_OFDM_CYCLIC_PREFIXER_IMPL_H
#define INCLUDED_MOFDM_OFDM_CYCLIC_PREFIXER_IMPL_H

#include <mofdm/ofdm_cyclic_prefixer.h>

namespace gr {
  namespace mofdm {

    class ofdm_cyclic_prefixer_impl : public ofdm_cyclic_prefixer
    {
     private:
      size_t d_fft_len;
      //! FFT length + CP length in samples
      size_t d_output_size;
      //! Length of the cyclic prefix in samples
      int d_cp_size;
      //! Length of pulse rolloff in samples
      int d_rolloff_len;
      //! Buffers the up-flank (at the beginning of the cyclic prefix)
      std::vector<float> d_up_flank;
      //! Buffers the down-flank (which trails the symbol)
      std::vector<float> d_down_flank;
      std::vector<gr_complex> d_delay_line; // We do this explicitly to avoid outputting zeroes (i.e. no history!)

     protected:
      int calculate_output_stream_length(const gr_vector_int &ninput_items);

     public:
      ofdm_cyclic_prefixer_impl(size_t input_size, size_t output_size, int rolloff_len, const std::string &len_tag_key);
      ~ofdm_cyclic_prefixer_impl();

      int work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace mofdm
} // namespace gr

#endif /* INCLUDED_MOFDM_OFDM_CYCLIC_PREFIXER_IMPL_H */

