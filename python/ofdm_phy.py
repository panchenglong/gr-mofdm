"""
OFDM PHY LAYER
"""
from gnuradio import gr
from gnuradio import blocks

import mofdm_swig as mofdm

# from current dir
from ofdm_txrx import ofdm_tx, ofdm_rx

class ofdm_phy(gr.hier_block2):
    def __init__(self):
        gr.hier_block2.__init__(
            self, "OFDM PHY",
            gr.io_signature(1, 1, gr.sizeof_gr_complex * 1),
            gr.io_signature2(2, 2, gr.sizeof_gr_complex * 1, gr.sizeof_char))
        
        # Message Port
        self.message_port_register_hier_out("from_mac")
        self.message_port_register_hier_in("to_mac")

        # Blocks
        self.tx_pdu = blocks.pdu_to_tagged_stream(blocks.byte_t, "packet_length")
        self.tx_path = ofdm_tx(scramble_bits=True)

        self.rx_path = ofdm_rx(scramble_bits=True)
        self.rx_pdu = blocks.tagged_stream_to_pdu(blocks.byte_t, "packet_length")

        # Connections
        self.connect(self.tx_pdu, self.tx_path, self)
        self.connect(self, self.rx_path, self.rx_pdu)
        self.connect((self.rx_path, 1), (self, 1))

        # Message Connection
        self.msg_connect(self, "from_mac", self.tx_pdu, "pdus")
        self.msg_connect(self.rx_pdu, "pdus", self, "to_mac")
