gr-mofdm
========

A GNU Radio ofdm tranceiver with a simple ARQ MAC implementation.


This module utilize the new OFDM implementaion shiped with GNU Radio v3.7 (i.e. code in gr-digital/python/digital/ofdm_txrx.py). Certain blocks have been replaced by the modified one, such as the ones with the same name with the original GNU Radio blocks.

The general arhitecture is motivated by Bastian Bloessl's [gr-ieee802-11](https://github.com/bastibl/gr-ieee802-11), and the ARQ implementaion stems from jmalsbury's [gr-mac](https://github.com/jmalsbury/gr-mac).
I have borrowed and modified many codes from these two works.

# Installation
GNU Radio v3.7 is needed, no other extra modules needed.

    git clone https://github.com/panchenglong/gr-mofdm.git
    cd gr-mofdm
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
    sudo ldconfig


# Running
Connect two machines A, B to their own usrp2. Then on machine A, do
    cd gr-mofdm/apps
    ./run-rx.sh

On machine B, do
    cd gr-mofdm/apps
    ./run-tx.sh

Then, on machine B, you can ping A with 
    ping 192.168.123.2
and on machine A, you can ping B with
    ping 192.168.123.1

The ouput is the sent and received message.

# More to say 
This work is in progress, SNR statistics and channel coding will be added soon. I will make the output infomation more readable first.

I think this work will replace the infamous tunnel.py in GNU Radio. It nearly drives me crazy.