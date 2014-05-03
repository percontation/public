#!/bin/sh
set -x

# delete existing settings
tc qdisc del dev eth0 root

# Setup a single root htb at 1:1, with basically unlimited bandwidth.
# I actually want WRR+TBF, but WRR isn't in mainline kernel and a
# single HTB may be more efficient anyways, even abused as it is.
tc qdisc add dev eth0 root handle 1: htb default 11
tc class add dev eth0 parent 1: classid 1:1 htb rate 1gbps cburst 1g

# Set up 3 classes:
#   1:10 for priority traffic
#   1:11 for general traffic (default)
#   1:12 for throttled traffic
tc class add dev eth0 parent 1:1 classid 1:10 htb prio 0 rate 100mbps ceil 1gbps  burst 10g cburst 1m
tc class add dev eth0 parent 1:1 classid 1:11 htb prio 3 rate   1mbps ceil 1gbps  burst 10g cburst 1m
tc class add dev eth0 parent 1:1 classid 1:12 htb prio 5 rate 3mbit   ceil 12mbps burst 10g cburst 1m

# Add traffic from http/https ports to the throttled traffic
{
	test -r /etc/nginx/nginx.conf && sed -En 's/^[ \t]*listen[ \t]+([0-9]+).*$/\1/p' /etc/nginx/nginx.conf;
	test -r /etc/lighttpd/lighttpd.conf && sed -En 's/^[ \t]*server.port[ \t]*=[ \t]*([0-9]+).*$/\1/p;s/^[ \t]*\$SERVER\["socket"\][ \t]*==.*:([0-9]+)".*$/\1/p' /etc/lighttpd/lighttpd.conf;
} | while read PORT
do tc filter add dev eth0 parent 1: protocol ip prio 1 u32 match ip sport "$(($PORT))" 0xffff flowid 1:12
done

# Add ssh/scp traffic to the priority traffic
SSHD_PORT="$((`sed -En 's/^[ \t]*Port[ \t]+([0-9]+)/\1/p' /etc/ssh/sshd_config`))"
tc filter add dev eth0 parent 1: protocol ip prio 1 u32 match ip sport "$SSHD_PORT" 0xffff flowid 1:10

# Priority traffic is interactive or file-transfer like, and not bittorrent or DDOS like.
# So, assuming correct marking, sfq will let priority traffic share well.
tc qdisc add dev eth0 parent 1:10 handle 12: sfq perturb 90

# We want to protect against surprises in general traffic, and BLUE is all about protecting
# against unresponsive flows. It's also very self-configuring, so we don't screw things up.
tc qdisc add dev eth0 parent 1:11 handle 13: sfb

# For throttled traffic, we're keen on fair sharing and shooting things in the head.
# Codel's focus on low delay is good for webserver. Parameters adjusted for less memory use.
tc qdisc add dev eth0 parent 1:12 handle 14: fq_codel limit 5000 flows 500 noecn