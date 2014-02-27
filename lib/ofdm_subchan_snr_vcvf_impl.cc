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

#include <gnuradio/expj.h>
#include <gnuradio/io_signature.h>
#include "ofdm_subchan_snr_vcvf_impl.h"

#define M_TWOPI (2*M_PI)

namespace gr {
  namespace mofdm {

    ofdm_subchan_snr_vcvf::sptr
    ofdm_subchan_snr_vcvf::make(int fft_len, int cp_len,
        //gr::digital::constellation_sptr constellation,
        const std::string& len_tag_key,    
        const std::vector<std::vector<int> > &occupied_carriers,
        const std::vector<std::vector<int> > &pilot_carriers ,
        const std::vector<std::vector<gr_complex> > &pilot_symbols,
        int symbols_skipped,
        bool input_is_shifted,
        bool debug)
    {
      return gnuradio::get_initial_sptr
        (new ofdm_subchan_snr_vcvf_impl(fft_len, cp_len, len_tag_key, occupied_carriers, 
          pilot_carriers, pilot_symbols, symbols_skipped, input_is_shifted, debug));
    }

    /*
     * The private constructor
     */
    ofdm_subchan_snr_vcvf_impl::ofdm_subchan_snr_vcvf_impl(int fft_len, int cp_len,
        //gr::digital::constellation_sptr constellation,  
        const std::string& len_tag_key,     
        const std::vector<std::vector<int> > &occupied_carriers,
        const std::vector<std::vector<int> > &pilot_carriers ,
        const std::vector<std::vector<gr_complex> > &pilot_symbols,
        int symbols_skipped,
        bool input_is_shifted,
        bool debug)
      : tagged_stream_block("ofdm_subchan_snr_vcvf",
              gr::io_signature::make(1, 1, fft_len * sizeof(gr_complex)),
              gr::io_signature::make(1, 1, fft_len * sizeof(float)),
              len_tag_key),
      d_fft_len(fft_len),
      d_cp_len(cp_len),
      d_occupied_carriers(fft_len, false),
      d_pilot_carriers(pilot_carriers.size(), std::vector<bool>(fft_len, false)),
      d_pilot_symbols(pilot_symbols.size(), std::vector<gr_complex>(fft_len, gr_complex(0, 0))),
      d_symbols_skipped(symbols_skipped),
      d_pilot_carr_set(pilot_carriers.empty() ? 0 : symbols_skipped % pilot_carriers.size()),
      d_num_pilot(pilot_carriers[0].size()),
      d_debug(debug),
      d_processed(0)
      //d_constellation(constellation)
    {
      d_rx_symbol_log = fopen("rx_symbol.dat", "w");

      int fft_shift_width = 0;
      if (input_is_shifted) {
        fft_shift_width = fft_len/2;
      }
      if (!occupied_carriers.size()) {
        std::fill(d_occupied_carriers.begin(), d_occupied_carriers.end(), true);
      } else {
        for (unsigned i = 0; i < occupied_carriers.size(); i++) {
          for (unsigned k = 0; k < occupied_carriers[i].size(); k++) {
            int carr_index = occupied_carriers[i][k];
            if (occupied_carriers[i][k] < 0) {
              carr_index += fft_len;
            }
            if (carr_index >= fft_len || carr_index < 0) {
              throw std::invalid_argument("data carrier index out of bounds.");
            }
            d_occupied_carriers[(carr_index + fft_shift_width) % fft_len] = true;
          }
        }
      }
      if (pilot_carriers.size()) {
        for (unsigned i = 0; i < pilot_carriers.size(); i++) {
          if (pilot_carriers[i].size() != pilot_symbols[i].size()) {
            throw std::invalid_argument("pilot carriers and -symbols do not match.");
          }
          for (unsigned k = 0; k < pilot_carriers[i].size(); k++) {
            int carr_index = pilot_carriers[i][k];
            if (pilot_carriers[i][k] < 0) {
              carr_index += fft_len;
            }
            if (carr_index >= fft_len || carr_index < 0) {
              throw std::invalid_argument("pilot carrier index out of bounds.");
            }
            d_pilot_carriers[i][(carr_index + fft_shift_width) % fft_len] = true;
            d_pilot_symbols[i][(carr_index + fft_shift_width) % fft_len] = pilot_symbols[i][k];
          }
        }
      }
    }

    /*
     * Our virtual destructor.
     */
    ofdm_subchan_snr_vcvf_impl::~ofdm_subchan_snr_vcvf_impl()
    {
      fclose(d_rx_symbol_log);
    }

    void
    ofdm_subchan_snr_vcvf_impl::parse_length_tags(
       const std::vector<std::vector<tag_t> >& tags,
       gr_vector_int& n_input_items_reqd
       )
    {
/*        if (d_fixed_frame_len) {
            n_input_items_reqd[0] = d_fixed_frame_len;
        } else {*/
            for (unsigned k = 0; k < tags[0].size(); k++) {
                if (tags[0][k].key == pmt::string_to_symbol(d_length_tag_key_str)) {
                    n_input_items_reqd[0] = pmt::to_long(tags[0][k].value);
                    remove_item_tag(0, tags[0][k]);
                }
            }
        //}
    }

    int
    ofdm_subchan_snr_vcvf_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      return 1;
    }


    int
    ofdm_subchan_snr_vcvf_impl::work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      if (d_debug) {
        std::cerr << "\n\n\n*****" << d_processed++ << ":\tIn ofdm_subchan_snr_vcvf_impl::general_work*****" << std::endl;
        std::cerr << "noutput_items = " << noutput_items << std::endl;
        std::cerr << "ninput_items size = " << ninput_items.size() << std::endl;
        std::cerr << "ninput_items[0] = " << ninput_items[0] << std::endl;
        std::cerr << "nitems_read = " << nitems_read(0) << std::endl;
      }

      const gr_complex *in = (const gr_complex *) input_items[0];
      float *snr = (float *) output_items[0];
      int carrier_offset = 0;
      int frame_len = 0;
      
      frame_len = ninput_items[0];

      // is frame len right????????????????
      if (frame_len == 0)
        return 0;

      std::vector<tag_t> tags;
      get_tags_in_range(tags, 0, nitems_read(0), nitems_read(0)+1);
      for (unsigned i = 0; i < tags.size(); i++) {
        if (pmt::symbol_to_string(tags[i].key) == "ofdm_sync_chan_taps") {
          d_channel_state = pmt::c32vector_elements(tags[i].value);
          remove_item_tag(0, tags[i]);
        }
        if (pmt::symbol_to_string(tags[i].key) == "ofdm_sync_carr_offset") {
          carrier_offset = pmt::to_long(tags[i].value);
        }
      }

      if (d_debug) {
        for (int i = 0; i < d_channel_state.size(); i++)
          std::cerr << "H[" << i << "] = " << d_channel_state[i] << std::endl;
        std::cerr << "carrier_offset = " << carrier_offset << std::endl;
      }

      // Copy the frame and the channel state vector such that the symbols are shifted to the correct position
      // gr_complex frame[d_fft_len * frame_len];//This works in c99 and GCC extension
      //gr_complex *frame = new gr_complex[d_fft_len * frame_len];
      boost::scoped_array<gr_complex> frame(new gr_complex[d_fft_len * frame_len]);
      gr_complex *raw_frame = frame.get();
      if (carrier_offset < 0) {
        memset((void *) raw_frame, 0x00, sizeof(gr_complex) * (-carrier_offset));
        memcpy(
            (void *) &raw_frame[-carrier_offset], (void *) in,
            sizeof(gr_complex) * (d_fft_len * frame_len + carrier_offset)
        );
      } else {
        memset((void *) (raw_frame + d_fft_len * frame_len - carrier_offset), 0x00, sizeof(gr_complex) * carrier_offset);
        memcpy(
            (void *) raw_frame, (void *) (in+carrier_offset),
            sizeof(gr_complex) * (d_fft_len * frame_len - carrier_offset)
        );
      }

/*      if (d_debug) {
        for (int i = 0; i < d_fft_len * frame_len; i++)
          std::cerr << "frame[" << i << "] = " << frame[i] << std::endl;
      }*/

      // Correct the frequency shift on the symbols
      gr_complex phase_correction;
      for (int i = 0; i < frame_len; i++) {
        phase_correction = gr_expj(-M_TWOPI * carrier_offset * d_cp_len / d_fft_len * (i+1));
        for (int k = 0; k < d_fft_len; k++) {
          frame[i*d_fft_len+k] *= phase_correction;
        }
      }

      /*******************************EVM****************************************/
      //boost::scoped_array<float> evm_snr(new float[d_fft_len]);
      boost::scoped_array<float> evm_tx(new float[d_fft_len]);
      
      for (int i = 0; i < d_fft_len; ++i)
      {
        //evm_snr[i] = 0.0;
        evm_tx[i] = 0.0;
      }

      float alpha = 0.1;

      gr_complex sym_eq, sym_est;
      /*std::vector<gr_complex> _constellation(2);
      _constellation[0] = gr_complex(-1, 0);
      _constellation[1] = gr_complex(1, 0);
*/
      float noise = 0.0 ;
      for (int i = 0; i < frame_len; i++) {
          for (int k = 0; k < d_fft_len; k++) {
              if (!d_pilot_carriers.empty() && d_pilot_carriers[d_pilot_carr_set][k]) {
                noise += std::norm(frame[i * d_fft_len + k]/d_channel_state[k]-d_pilot_symbols[d_pilot_carr_set][k]);
                //std::cerr << "noise = " << noise << std::endl;
                d_channel_state[k] = alpha * d_channel_state[k]
                   + (1 - alpha) * frame[i * d_fft_len + k] / d_pilot_symbols[d_pilot_carr_set][k];
                //frame[i * d_fft_len + k] = d_pilot_symbols[d_pilot_carr_set][k];
              }
          }
          d_pilot_carr_set = (d_pilot_carr_set + 1) % d_pilot_carriers.size();

      } 
      for (int i = 0; i < frame_len; i++) {
          for (int k = 0; k < d_fft_len; k++) {
              if (!d_occupied_carriers[k]) {
                  continue;
              }
              sym_eq = frame[i * d_fft_len + k] / d_channel_state[k];
              evm_tx[k] += std::norm(sym_eq);
              //d_constellation->map_to_points(d_constellation->decision_maker(&sym_eq), &sym_est);
              /*unsigned int bits =  real(sym_eq) > 0;
              sym_est = _constellation[bits];
              d_channel_state[k] = alpha * d_channel_state[k]
                 + (1 - alpha) * frame[i * d_fft_len + k] / sym_est;*/
              //frame[i * d_fft_len + k] = sym_est;
              //evm_snr[k] += std::norm(sym_eq - sym_est);
          }
      }

      noise /= frame_len * 4;
      std::cerr << "noise = " << noise << std::endl;
      std::cerr << "\n\nprocessing " << d_processed++ << " packet\n";
      std::cerr << "{";
      
      for (int k = 0; k < d_fft_len; k++) {
        if (!d_occupied_carriers[k]) {
                  continue;
        }
        //evm_snr[k] /= frame_len;
        evm_tx[k] /= frame_len;
        
        //std::cerr << "evm_snr[" << k << "] = " << 10 * log10(1 / evm_snr[k]) << std::endl;
        std::cerr << 10 * log10(evm_tx[k] / noise) << ", ";
      }
      std::cerr << "}\n\n";
      /*******************************end EVM****************************************/
      d_pilot_carr_set = 0;
      // log received data, in mathmetica list style.
      if (true) {
        fprintf(d_rx_symbol_log, "\n\n%d th processing packet(0 based)\n", d_processed - 1);
        for (int i = 0; i < frame_len; i++) {
          fprintf(d_rx_symbol_log, "%d th symbol\n", i);
          fprintf(d_rx_symbol_log, "{ ");
          for (int j = 0; j < d_fft_len; ++j) {
            if (d_occupied_carriers[j] == true) {
              fprintf(d_rx_symbol_log, "%f+(%f)I, ", frame[j + i * d_fft_len].real(),
                frame[j + i * d_fft_len].imag());
            }
          }
          fprintf(d_rx_symbol_log, "}\n\n");
        }
      }


      // Calculate the average receive power of every subchannel.
      std::vector<float> rx_pow(d_fft_len, 0.0);
      for (int i = 0; i < d_fft_len; i++) {
        if (d_occupied_carriers[i] == false) 
          continue;
        float cumsum = 0.0;
        for (int j = 0; j < frame_len; j++) {
          cumsum += std::norm(frame[i + j * d_fft_len]);
        }
        rx_pow[i] = cumsum / frame_len;
        if (d_debug) {
          std::cerr << "rx_pow[" << i << "] = " << rx_pow[i] << std::endl;
        }
      }

      // Estimate N0 using pilot info.
      float cumsum = 0.0;

      for (int i = 0; i < frame_len; i++) {
        for (int j = 0; j < d_fft_len; j++) {
          if (d_pilot_carriers[d_pilot_carr_set][j] == false)
            continue;
          if (d_debug) {
            std::cerr << "rx pilot = " << frame[j + i * d_fft_len] <<
              "\tchan state = " <<  d_channel_state[j] << 
              "\tpilot sym = " << d_pilot_symbols[d_pilot_carr_set][j] << std::endl;
          }
          cumsum += std::norm(frame[j + i * d_fft_len] - d_channel_state[j] * d_pilot_symbols[d_pilot_carr_set][j]);
        }
        d_pilot_carr_set = (d_pilot_carr_set + 1) % d_pilot_carriers.size();
      }
      float N0 = cumsum / (frame_len * d_num_pilot);

      if (d_debug) {
          std::cerr << "N0 = " << N0 << std::endl;
          //std::cerr << "fuckN0 = " << fuckN0 << std::endl;
      }

      // Calculate the snr (in db) in every subchannel.
      for (int i = 0; i < d_fft_len; i++) {
        snr[i] = 10 * log10(rx_pow[i] / N0 - 1);
        //evm_snr[i] = evm_snr[i] / frame_len;
        if (d_debug) {
          std::cerr << "snr[" << i << "] = " << snr[i] << std::endl;
          //std::cerr << "fucksnr[" << i << "] = " << 10 * log10(rx_pow[i] / fuckN0) << std::endl;
          //std::cerr << "evm_snr[" << i << "] = " << 10 * log10(1.0 / evm_snr[i]) << std::endl;
        }
      }

      // Tell runtime system how many input items we consumed on
      // each input stream.
      //consume_each (frame_len);

      // Tell runtime system how many output items we produced.
      return 1;
    }

  } /* namespace mofdm */
} /* namespace gr */

