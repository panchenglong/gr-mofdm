from gnuradio import gr, blocks
import pmt
import gnuradio.gr.gr_threading as _threading

import struct, sys, time

class test_app(gr.basic_block):
    def __init__(self, options):
        gr.basic_block.__init__(self,
            name="test_app",
            in_sig=None,
            out_sig=None)

        self.options = options

        self.mac_addr = tuple(int(i, 16) for i in self.options.rx_addr.split(':'))

        self.message_port_register_out(pmt.intern('to_mac'))
        self.thread = _threading.Thread(target=self.generate_msg)
        self.thread.start()

        self.message_port_register_in(pmt.intern('from_mac'))
        self.set_msg_handler(pmt.intern('from_mac'),self.received_msg)

        self.message_port_register_in(pmt.intern('monitor_in'))
        self.set_msg_handler(pmt.intern('monitor_in'),self.monitor_msg)

    def generate_msg(self):
        if self.options.receive_only:
            return

        nbytes = int(1e6 * self.options.megabytes)
        n = 0
        pktno = 0
        pkt_size = int(self.options.size)

        while n < nbytes:
            data = (pkt_size - 2) * chr(pktno & 0xff) 
            payload = struct.pack('!H', pktno & 0xffff) + data

            time.sleep(0.001)
            self.send_pkt(payload)
            n += len(payload)
            sys.stderr.write('.')
            pktno += 1

        self.send_eof()

    def send_pkt(self, payload):
        data = pmt.init_u8vector(len(payload), [ord(c) for c in payload])
        meta_dict = {}
        meta_dict['dst_addr'] = pmt.init_u8vector(len(self.mac_addr), self.mac_addr)
        meta = pmt.to_pmt(meta_dict)
        self.message_port_pub(pmt.intern('to_mac'), pmt.cons(meta,data)) 

    def send_eof(self):
        self.message_port_pub(pmt.intern('to_mac'), pmt.PMT_EOF)

    def received_msg(self, msg):
        meta = pmt.car(msg)
        meta_dict = pmt.to_python(meta)
        print >> sys.stderr, 'seq no. =', meta_dict['seq_nr']
        data = pmt.cdr(msg)
        data = pmt.u8vector_elements(data)
        print >> sys.stderr, 'received data:\n', data
        print >> sys.stderr, '\n'
    
    def monitor_msg(self, msg):
        if not pmt.is_dict(msg):
            raise TypeError('monitor msg should be type pmt dict')

        info = pmt.to_python(msg)
        for key, value in info.items():
            print >> sys.stderr, key, ':', value

        print >> sys.stderr, '\n'
        
