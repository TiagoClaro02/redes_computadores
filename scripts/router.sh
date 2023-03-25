/system reset-configuration
y
/ip firewall nat enable 0
/ip address add address=172.16.1.49/24 interface=ether1
/ip address add address=172.16.41.254/24 interface=ether2
/ip route add dst-address=172.16.40.0/24 gateway=172.16.41.253
/ip route add dst-address=0.0.0.0/0 gateway=172.16.2.254
