sudo systemctl restart networking
ifconfig eth0 172.16.41.1/24
route add -net 172.16.40.0/24 gw 172.16.41.253
route add default gw 172.16.41.254
echo 0 > /proc/sys/net/ipv4/conf/eth0/accept_redirects
echo 0 > /proc/sys/net/ipv4/conf/all/accept_redirects
route del -net 172.16.40.0/14 gw 172.16.41.253
route add -net 172.16.40.0/24 gw 172.16.41.253
echo 1 > /proc/sys/net/ipv4/conf/eth0/accept_redirects
echo 1 > /proc/sys/net/ipv4/conf/all/accept_redirects
