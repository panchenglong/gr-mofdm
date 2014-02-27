/* -*- c++ -*- */

#define MOFDM_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "mofdm_swig_doc.i"

%{
#include "mofdm/ofdm_subchan_snr_vcvf.h"
#include "mofdm/ofdm_data_sink.h"
#include "mofdm/rx_sample_metadata.h"
#include "mofdm/ofdm_cyclic_prefixer.h"
#include "mofdm/ofdm_mac.h"
%}


%include "mofdm/ofdm_subchan_snr_vcvf.h"
GR_SWIG_BLOCK_MAGIC2(mofdm, ofdm_subchan_snr_vcvf);
%include "mofdm/ofdm_data_sink.h"
GR_SWIG_BLOCK_MAGIC2(mofdm, ofdm_data_sink);


%include "mofdm/rx_sample_metadata.h"
GR_SWIG_BLOCK_MAGIC2(mofdm, rx_sample_metadata);

%include "mofdm/ofdm_cyclic_prefixer.h"
GR_SWIG_BLOCK_MAGIC2(mofdm, ofdm_cyclic_prefixer);
%include "mofdm/ofdm_mac.h"
GR_SWIG_BLOCK_MAGIC2(mofdm, ofdm_mac);

