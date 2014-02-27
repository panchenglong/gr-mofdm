#!/bin/bash

### create tap interface
if [[ `ifconfig -a | grep tap0 | wc -l` -eq 0 ]]
then
	sudo ip tuntap add dev tap0 user ${USER} mode tap
fi

### reconfigure it in any case, just to be sure it's up
sudo ifconfig tap0 down
sudo ifconfig tap0 hw ether 48:89:67:45:23:01
sudo ifconfig tap0 mtu 256
sudo ifconfig tap0 up
sudo ifconfig tap0 192.168.123.2

sudo route del -net 192.168.123.0/24
sudo route add -net 192.168.123.0/24 mss 400 dev tap0

sudo tc qdisc del dev tap0 root
sudo tc qdisc add dev tap0 root netem delay 10ms

sudo arp -s 192.168.123.1 48:23:45:67:89:ab


### start transceiver
TRANSCEIVER="../examples/ofdm_transceiver.py"
rm -rf *.dat
sudo LD_LIBRARY_PATH=/usr/local/lib ./${TRANSCEIVER} --receive-only --tx-addr=48:89:67:45:23:01 --rx-addr=48:23:45:67:89:ab 2> >(tee log2 >&2)
