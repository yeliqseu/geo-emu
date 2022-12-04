# GEO SatCom Emulation Using *sns3* and Linux TAP

This is a tutorial on building a GEO satellite communication (SatCom) emulation enivronment by combining the ns-3 SatCom module *sns3* (https://sns3.org/) and Linux TAP devices. Through ns-3's TapBridge module, the following procedure can transform a Linux host with multiple NICs to a SatCom emulation system where two NICs emulate the entry of the User Terminal (UT) and Gateway (GW) of the system, respectively. Then, one can pass real-life data traffic through the two NICs, and the traffic will traverse the GEO system emulated by *sns3*.

The emulation environment is conceived when testing HTTP adaptive streaming over SatCom in the following paper

> Y. Li, R. Feng, R. Gao and J. Wang, "Fountain Coded Streaming for SAGIN with Learning-based Pause-and-Listen," in _IEEE Networking Letters_, 2022, _accepted_.

Please kindly cite the work if you find the environment useful.

## 1, Download ns-3.35 and install sns3

`sns3` was developed several years ago (before ns-3.29). It cannot be directly installed in ns-3 newer than 3.31 due to API changes. To fix this, a compatible sns3 ported to ns-3.35 is provided by the author in the following GitHub repository:

https://github.com/yeliqseu/tcp-lte-sat

Please clone it and then install necessary sns-3 supplement modules, including sns3-data, magister-traffic, and magister-stats.

## 2, ns-3 simulation script

The emulated environment is specified in `sat-tap-emu.cc`. Simply put it in `ns-3.35/scratch/`. Note that the script specifies the user-terminal (UT) side as `10.10.0.1/16`, and the subnet on the gateway(GW) side as `10.31.0.1/16`. Modify them if needed. The routing within the GEO link is handled automatically by sns3.

## 3, Setup GEO emulation node

Run the following commands on a multi-home host (with at least two ethernet interfaces, e.g. eth1 and eth2) to setup Linux TAP devices and bridges:

```
sudo ip tuntap add mode tap utuser-tap
sudo ip tuntap add mode tap gwuser-tap
sudo ip link add utside-br type bridge
sudo ip link add gwside-br type bridge
sudo ip link set utuser-tap up
sudo ip link set gwuser-tap up
sudo ip link set eth1 master utside-br
sudo ip link set utuser-tap master utside-br
sudo ip link set eth2 master gwside-br
sudo ip link set gwuser-tap master gwside-br
sudo ip addr add 10.10.0.254/16 dev utside-br
sudo ip addr add 10.31.0.254/16 dev gwside-br
sudo ip link set utside-br up
sudo ip link set gwside-br up
```

Note that the bridges' addresses match those of UT-tap and GW-tap in sat-tap-emu.cc. Here we choose `10.10.0.254` and `10.31.0.254`, respectively. 

Given the above settings, a host in `10.10.0.0/16` (say client with IP `10.10.0.100`) connecting to eth1 can reach UT of the sns3 node, while a host in `10.31.0.0/16` connecting to eth2 (say server with IP `10.31.0.100`) can reach GW.

## 4, Add routing rules to the client and server hosts

client to server:

>    sudo ip route add 10.31.0.0/16 via 10.10.0.1

server to client:

>    sudo ip route add 10.10.0.0/16 via 10.31.0.1

## 5, Verify the environment

On client:
> ping 10.31.0.100
>
>traceroute 10.31.0.100

On server:

> ping 10.10.0.100
>
> traceroute 10.10.0.100
