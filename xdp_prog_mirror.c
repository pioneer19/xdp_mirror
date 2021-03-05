/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/bpf.h>
#include <linux/in.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

// The parsing helper functions from the packet01 lesson have moved here
#include "parsing_helpers.h"
#include "rewrite_helpers.h"

/* Defines xdp_stats_map */
#include "xdp_stats_kern_user.h"
#include "xdp_stats_kern.h"

#ifndef memcpy
#define memcpy(dest, src, n) __builtin_memcpy((dest), (src), (n))
#endif

/*
 * UDP echo to test network performance
 * will switch ethernet MAC addresses, UDP ports and send packet back
 */
SEC("xdp_udp_echo")
int xdp_udp_echo_func(struct xdp_md *ctx)
{
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;
    struct hdr_cursor nh;
    struct ethhdr* packet_eth_header;
    int eth_type;
    int ip_type;
    struct iphdr*   ipv4_header;
    struct ipv6hdr* ipv6_header;
    struct udphdr* udp_header;
    __u32 action = XDP_PASS;

    /* These keep track of the next header type and iterator pointer */
    nh.pos = data;

    /* Parse Ethernet and IP/IPv6 headers */
    eth_type = parse_ethhdr(&nh, data_end, &packet_eth_header);
    if (eth_type == bpf_htons(ETH_P_IP)) {
        ip_type = parse_iphdr(&nh, data_end, &ipv4_header);
        if (ip_type != IPPROTO_UDP)
            goto out;
    } else if (eth_type == bpf_htons(ETH_P_IPV6)) {
        ip_type = parse_ip6hdr(&nh, data_end, &ipv6_header);
        if (ip_type != IPPROTO_UDP)
            goto out;
    } else {
        goto out;
    }

    if( nh.pos + sizeof(*udp_header) > data_end )
        goto out;
    udp_header = nh.pos;

    /* Swap Ethernet source and destination */
    swap_src_dst_mac(packet_eth_header);

    if (eth_type == bpf_htons(ETH_P_IP)) {
        swap_src_dst_ipv4(ipv4_header);
    } else {
        swap_src_dst_ipv6(ipv6_header);
    }
    swap_udp_ports(udp_header);

    action = XDP_TX;
out:
    return xdp_stats_record_action(ctx, action);
}

char _license[] SEC("license") = "GPL";
