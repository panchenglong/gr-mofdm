#!/usr/bin/python
#!/usr/bin/env python

from gnuradio import gr
from gnuradio import eng_notation
from gnuradio.eng_option import eng_option
from optparse import OptionParser

from gnuradio import uhd
from gnuradio import blocks
import mofdm

# /////////////////////////////////////////////////////////////////////////////
#                             the flow graph
# /////////////////////////////////////////////////////////////////////////////

class my_top_block(gr.top_block):
    def __init__(self, options):
        gr.top_block.__init__(self)

        self.options = options

        # Blocks
        #self.app = test_app(self.options)
        self.tap= blocks.tuntap_pdu("tap0", 256)
        self.encap = mofdm.ofdm_mac_encap(self.options)

        self.mac = mofdm.ofdm_mac(self.options.tx_addr, 
                                  self.options.ack_timeout, 
                                  self.options.max_attempts,
                                  self.options.debug)

        self.phy = mofdm.ofdm_phy()
        self.scale = blocks.multiply_const_vcc((0.05, ))

        self.sink = uhd.usrp_sink(
            device_addr="",
            stream_args=uhd.stream_args(
                cpu_format="fc32",
                channels=range(1),
            ),
        )
        self.sink.set_samp_rate(self.options.sample_rate)
        self.sink.set_center_freq(self.options.tx_freq, 0)
        self.sink.set_gain(self.options.tx_gain, 0)
        self.sink.set_antenna("TX/RX", 0)
    
        self.src = uhd.usrp_source(
            device_addr="",
            stream_args=uhd.stream_args(
                cpu_format="fc32",
                channels=range(1),
            ),
        )
        self.src.set_samp_rate(self.options.sample_rate)
        self.src.set_center_freq(self.options.rx_freq, 0)
        self.src.set_gain(self.options.rx_gain, 0)
        self.src.set_antenna("TX/RX", 0)

        # Connections
        self.connect(self.phy, self.scale, self.sink)
        self.connect(self.src, self.phy)
        self.connect((self.phy, 1), self.mac)

        #if self.options.receive_only:
        #    self.connect(self.src, blocks.complex_to_mag_squared(1), 
        #            blocks.file_sink(gr.sizeof_float, "rx_sample.dat"))

        # Message Connections
        self.msg_connect(self.mac, "to_phy", self.phy, "from_mac")
        self.msg_connect(self.phy, "to_mac", self.mac, "from_phy")
        
        self.msg_connect(self.tap, "pdus", self.encap, "from_tap")
        self.msg_connect(self.encap, "to_tap", self.tap, "pdus")
        self.msg_connect(self.encap, "to_mac", self.mac, "from_encap")
        self.msg_connect(self.mac, "to_encap", self.encap, "from_mac")

        self.msg_connect(self.mac, "monitor_out", self.encap, "monitor_in")


# /////////////////////////////////////////////////////////////////////////////
#                                   main
# /////////////////////////////////////////////////////////////////////////////

def main():

    parser = OptionParser(option_class=eng_option, conflict_handler="resolve")


    parser.add_option("", "--tx-freq", type="eng_float", default=900e6,
                      help="set receive frequency to FREQ [default=%default]",
                      metavar="FREQ")
    parser.add_option("", "--rx-freq", type="eng_float", default=900e6,
                      help="set receive frequency to FREQ [default=%default]",
                      metavar="FREQ")
    parser.add_option("", "--sample-rate", type="eng_float", default=10e6,
                      help="sample rate [default=%default]")
    parser.add_option("-s", "--size", type="eng_float", default=100,
                      help="set packet size [default=%default]")
    parser.add_option("-M", "--megabytes", type="eng_float", default=1.0,
                      help="set megabytes to transmit [default=%default]")
    parser.add_option("", "--ack-timeout", type="int", default=500,
                      help="set ack timeout, in milliseconds [default=%default]")
    parser.add_option("", "--tx-gain", type="eng_float", default=25,
                          help="set transmit gain in dB [default=%default]")
    parser.add_option("", "--rx-gain", type="eng_float", default=20,
                          help="set receive gain in dB [default=%default]")
    parser.add_option("", "--probe-threshold", type="eng_float", default=10,
                          help="probe threshold in dB [default=%default]")
    parser.add_option("", "--tx-addr", type="string", default="01:23:45:67:89:ab",
                          help="sender address [default=%default]")
    parser.add_option("", "--rx-addr", type="string", default="ab:89:67:45:23:01",
                          help="receiver address [default=%default]")
    parser.add_option("", "--max-attempts", type="int", default=100,
                          help="max retransmit attempts [default=%default]")
    parser.add_option("", "--receive-only", action="store_true", default=False,
                      help="Only receive packets")
    parser.add_option("", "--debug", action="store_true", default=False,
                      help="debug")

    (options, args) = parser.parse_args ()

    # Attempt to enable realtime scheduling
    r = gr.enable_realtime_scheduling()
    if r == gr.RT_OK:
        realtime = True
    else:
        realtime = False
        print "Note: failed to enable realtime scheduling"


    # build the graph (PHY)
    tb = my_top_block(options)

    tb.start()    # Start executing the flow graph (runs in separate threads)

    #tb.stop()     # but if it does, tell flow graph to stop.
    tb.wait()     # wait for it to finish
                

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
