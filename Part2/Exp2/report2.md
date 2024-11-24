# Exp 2 - Implement two bridges in a switch


## Steps

1. Connect and configure E1 of tux12.

2. Register its IP and MAC addresses.

AAAA

3. Create two bridges in the switch: bridgeY0 and bridgeY1.

```c
[admin@MikroTik] > /interface bridge add name=bridgeY0
[admin@MikroTik] > /interface bridge add name=bridgeY1
```

4. Remove the ports where tuxY2, tuxY3 and tuxY4 are connected from the default bridge (bridge).

```c 
[admin@MikroTik] > /interface bridge port remove [find interface =ether18]
[admin@MikroTik] > /interface bridge port remove [find interface =ether10]
[admin@MikroTik] > /interface bridge port remove [find interface =ether9]
```

5. Add the corresponding ports of tuxY2 and tuxY3 to bridgeY0, and tuxY4 to bridgeY1

```c
[admin@MikroTik] > /interface bridge port add bridge=bridgeY0 interface=ether9 
[admin@MikroTik] > /interface bridge port add bridge=bridgeY0 interface=ether10
[admin@MikroTik] > /interface bridge port add bridge=bridgeY1 interface=ether18 
```

6. Start the capture at tuxY3.eth1

7. In tuxY3, ping tuxY4 and then ping tuxY2

```c
root@gnu13:~# ping -b 172.16.10.254 #Tux14  -> Normal

root@gnu13:~# ping -b 172.16.11.1 #Tux12 -> No conection
```

8. Stop the capture and save the log

![WireShark Screenshot Step 6 - tux ](.png)

9. Start new captures in tuxY2.eth1, tuxY3.eth1 and tuxY4.eth1

10. In tuxY3, do ping broadcast for a few seconds

```c
root@gnu13:~# ping -b 172.16.10.255
```

11. Observe the results, stop the captures and save the logs

![WireShark Screenshot Step 9 - tux ](.png)

![WireShark Screenshot Step 9 - tux ](.png)

![WireShark Screenshot Step 9 - tux ](.png)

12. Repeat steps 7, 8 and 9, but now do ping broadcast in tuxY2 

```c
root@gnu12:~# ping -b 172.16.11.255
```

![WireShark Screenshot Step 10 - tux ](.png)

![WireShark Screenshot Step 10 - tux ](.png)

![WireShark Screenshot Step 10 - tux ](.png)


##  Questions

- **How to configure bridgeY0?**

We configured bridgeY0 on the switch to establish a connection between the computers tuxY3 and tuxY4 (Step 3), creating a subnet.

We removed the default bridge configuration applied by the switch to the ports of all the computers tuxY2, tux3 and tuxY4 (Step 4). Then add the ports associated with tuxY3 and tuxY4 to the created bridgeY0 (Step 5).

The commands used in the switch interface to configure the bridge are detailed in the Steps above.


- **How many broadcast domains are there? How can you conclude it from the logs?**



