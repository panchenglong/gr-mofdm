from gnuradio import gr, blocks
import pmt

import struct, sys, time

class ofdm_mac_encap(gr.basic_block):
    def __init__(self, options):
        gr.basic_block.__init__(self,
            name="ofdm_mac_encap",
            in_sig=None,
            out_sig=None)

        self.options = options

        self.dst_addr = tuple(int(i, 16) for i in self.options.rx_addr.split(':'))

        self.message_port_register_out(pmt.intern('to_tap'))

        self.message_port_register_in(pmt.intern('from_tap'))
        self.set_msg_handler(pmt.intern('from_tap'), self.rx_from_tap)

        self.message_port_register_out(pmt.intern('to_mac'))

        self.message_port_register_in(pmt.intern('from_mac'))
        self.set_msg_handler(pmt.intern('from_mac'),self.rx_from_mac)

        self.message_port_register_in(pmt.intern('monitor_in'))
        self.set_msg_handler(pmt.intern('monitor_in'),self.monitor_msg)

    def rx_from_tap(self, msg):
        meta = pmt.car(msg)
        meta = pmt.dict_add(meta, pmt.intern('dst_addr'), pmt.init_u8vector(len(self.dst_addr), self.dst_addr))
        self.message_port_pub(pmt.intern('to_mac'), pmt.cons(meta, pmt.cdr(msg)))

    def send_eof(self):
        self.message_port_pub(pmt.intern('to_mac'), pmt.PMT_EOF)

    def rx_from_mac(self, msg):
        meta = pmt.car(msg)
        meta_dict = pmt.to_python(meta)
        print >> sys.stderr, 'seq no. =', meta_dict['seq_nr']
        data = pmt.cdr(msg)
        print >> sys.stderr, 'received data:\n', pmt.u8vector_elements(data)
        print >> sys.stderr, '\n'

        self.message_port_pub(pmt.intern('to_tap'), pmt.cons(pmt.PMT_NIL, data))
    
    def monitor_msg(self, msg):
        if not pmt.is_dict(msg):
            raise TypeError('monitor msg should be type pmt dict')

        info = pmt.to_python(msg)
        for key, value in info.items():
            print >> sys.stderr, key, ':', value

        print >> sys.stderr, '\n'
        
