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

#ifndef INCLUDED_MOFDM_OFDM_SUBCHAN_SNR_VCVF_IMPL_H
#define INCLUDED_MOFDM_OFDM_SUBCHAN_SNR_VCVF_IMPL_H

#include <mofdm/ofdm_subchan_snr_vcvf.h>
#include <cstdlib>
#include <cstdio>

namespace gr {
  namespace mofdm {

    class ofdm_subchan_snr_vcvf_impl : public ofdm_subchan_snr_vcvf
    {
     private:
      int d_fft_len;
      int d_cp_len;
      //! If \p d_occupied_carriers[k][l] is true, symbol k, carrier l is carrying data.
      //  (this is a different format than occupied_carriers!)
      std::vector<bool> d_occupied_carriers;
      //! If \p d_pilot_carriers[k][l] is true, symbol k, carrier l is carrying data.
      //  (this is a different format than pilot_carriers!)
      std::vector<std::vector<bool> > d_pilot_carriers;
      //! If \p d_pilot_carriers[k][l] is true, d_pilot_symbols[k][l] is its tx'd value.
      //  (this is a different format than pilot_symbols!)
      std::vector<std::vector<gr_complex> > d_pilot_symbols;
      //! In case the frame doesn't begin with OFDM symbol 0, this is the index of the first symbol
      int d_symbols_skipped;
      //! The current position in the set of pilot symbols
      int d_pilot_carr_set;
      //! Vector of length d_fft_len saving the current channel state (on the occupied carriers)
      std::vector<gr_complex> d_channel_state;

      int d_num_pilot;
      bool d_debug;

      int d_processed; // number of packets processed, used for debuging
      FILE *d_rx_symbol_log;

      //gr::digital::constellation_sptr d_constellation;

     protected:
      void parse_length_tags(
         const std::vector<std::vector<tag_t> >& tags,
         gr_vector_int& n_input_items_reqd
         );

      int calculate_output_stream_length(const gr_vector_int &ninput_items);


     public:
      ofdm_subchan_snr_vcvf_impl(int fft_len, int cp_len,
        //gr::digital::constellation_sptr constellation,       
        const std::string& len_tag_key,
        const std::vector<std::vector<int> > &occupied_carriers,
        const std::vector<std::vector<int> > &pilot_carriers ,
        const std::vector<std::vector<gr_complex> > &pilot_symbols,
        int symbols_skipped,
        bool input_is_shifted,
        bool debug);
      ~ofdm_subchan_snr_vcvf_impl();

      // Where all the action really happens

      int work(int noutput_items,
               gr_vector_int& ninput_items,
               gr_vector_const_void_star& input_items,
               gr_vector_void_star& output_items);
    };

  } // namespace mofdm
} // namespace gr

#endif /* INCLUDED_MOFDM_OFDM_SUBCHAN_SNR_VCVF_IMPL_H */

