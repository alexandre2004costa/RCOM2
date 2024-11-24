# Exp 1 - Configure an IP Network

## Steps

1. Connect E1 of tux13 and E1 of tux14 to the switch

2. Configure eth1 interface of tux13 and eth1 interface of tux14 using ifconfig and route commands

```c
root@gnu13:~# ifconfig eth1 up
root@gnu13:~# ifconfig eth1 172.16.10.1/24
root@gnu13:~# ifconfig
eth1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.16.10.1  netmask 255.255.255.0  broadcast 172.16.10.255
        inet6 fe80::208:54ff:fe71:73c6  prefixlen 64  scopeid 0x20<link>
        ether 00:08:54:71:73:c6  txqueuelen 1000  (Ethernet)
        RX packets 8  bytes 480 (480.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 22  bytes 3164 (3.0 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

```c
root@gnu14:~# ifconfig eth1 172.16.10.254/24
root@gnu14:~# ifconfig
eth1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.16.10.254  netmask 255.255.0.0  broadcast 172.16.255.255
        inet6 fe80::2c0:dfff:fe08:d5b0  prefixlen 64  scopeid 0x20<link>
        ether 00:c0:df:08:d5:b0  txqueuelen 1000  (Ethernet)
        RX packets 227  bytes 29239 (28.5 KiB)
        RX errors 0  dropped 103  overruns 0  frame 0
        TX packets 77  bytes 9484 (9.2 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

```

3. Register the IP and MAC addresses of the network interfaces

AAAAAAAAAAAAAAAAA

4. Use ping command to verify connectivity between these computers

```c
root@gnu14:~# ping 172.16.10.1
PING 172.16.10.1 (172.16.10.1) 56(84) bytes of data.
64 bytes from 172.16.10.1: icmp_seq=1 ttl=64 time=0.188 ms
```

5. Inspect forwarding (route -n) and ARP (arp -a) tables

```c
root@gnu13:~# route -n
Kernel IP routing table
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
0.0.0.0         10.227.20.254   0.0.0.0         UG    0      0        0 eth0
10.227.20.0     0.0.0.0         255.255.255.0   U     0      0        0 eth0
172.16.10.0     0.0.0.0         255.255.255.0   U     0      0        0 eth1

root@gnu13:~# arp -a
? (172.16.10.254) at 00:c0:df:08:d5:b0 [ether] on eth1
? (10.227.20.254) at e4:8d:8c:20:25:c8 [ether] on eth0
```

6. Delete ARP table entries in tuxY3 (arp -d ipaddress)

```c
root@gnu13:~# sudo arp -d 172.16.10.254   # Empty
```

7. Start Wireshark in tuxY3.eth1 and start capturing packets

8. In tuxY3, ping tuxY4 for a few seconds

```c
root@gnu13:~# ping 172.16.10.254
```

9. Stop capturing packets

10. Save the log and study it

![WireShark Screenshot](.png)


## Questions
- **What are the ARP packets and what are they used for?**

The ARP (Address Resolution Protocol) packets are used in IP networks to map an IP address to a MAC address.

- **What are the MAC and IP addresses of ARP packets and why?**

MAC: 
IP: 


- **What packets does the ping command generate?**

The ping command generates ICMP (Internet Control Message Protocol) packets.

- **What are the MAC and IP addresses of the ping packets?**

The MAC and IP addresses of the ICMP packets used on ping are 

- **How to determine if a receiving Ethernet frame is ARP, IP, ICMP?**

An Ethernet frame contains an EtherType field that identifies the protocol of the encapsulated payload. 
In Wireshark, the protocol of each frame is displayed in the "Protocol" column of the capture.


- **How to determine the length of a receiving frame?**

FALTA
In Wireshark, the length in bytes for each frame is displayed in the "Length" column of the capture.


- **What is the loopback interface and why is it important?**

