#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

#define INTERFACE_NAME_SIZE 32
#define MAX_DEVICES 100

struct net_interface {
    char name[INTERFACE_NAME_SIZE];
    unsigned long rx_bytes; // Received bytes
    unsigned long tx_bytes; // Transmitted bytes
};

struct device_usage {
    u_char mac[6];
    unsigned long rx_bytes;
    unsigned long tx_bytes;
};

struct device_usage device_stats[MAX_DEVICES];
int device_count = 0;

// Function to parse /proc/net/dev and get network stats
void get_network_stats(struct net_interface *net_stat, const char *interface_name) {
    FILE *fp;
    char line[256];

    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL) {
        perror("Error opening /proc/net/dev");
        exit(EXIT_FAILURE);
    }

    // Skip the first two lines (headers)
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        char iface[INTERFACE_NAME_SIZE];
        unsigned long rx_bytes, tx_bytes;

        // Extract network interface name and received/transmitted bytes
        sscanf(line, "%31s %lu %*d %*d %*d %*d %*d %*d %lu", iface, &rx_bytes, &tx_bytes);

        // Remove the colon (":") from interface name
        char *colon = strchr(iface, ':');
        if (colon) *colon = '\0';

        // Check if it's the interface we are looking for
        if (strcmp(iface, interface_name) == 0) {
            strcpy(net_stat->name, iface);
            net_stat->rx_bytes = rx_bytes;
            net_stat->tx_bytes = tx_bytes;
            break;
        }
    }

    fclose(fp);
}

// Function to calculate bandwidth usage for the interface
void calculate_bandwidth_usage(const struct net_interface *old_stat, const struct net_interface *new_stat, double interval) {
    unsigned long rx_diff = new_stat->rx_bytes - old_stat->rx_bytes;
    unsigned long tx_diff = new_stat->tx_bytes - old_stat->tx_bytes;

    double rx_rate = (double)rx_diff / interval; // Bytes per second (Rx)
    double tx_rate = (double)tx_diff / interval; // Bytes per second (Tx)

    printf("Interface: %s\n", new_stat->name);
    printf("Download rate: %.2f KB/s\n", rx_rate / 1024.0);
    printf("Upload rate: %.2f KB/s\n", tx_rate / 1024.0);
}

// Function to find device index or add a new device
int find_or_add_device(const u_char *mac_addr) {
    for (int i = 0; i < device_count; i++) {
        if (memcmp(device_stats[i].mac, mac_addr, 6) == 0) {
            return i;  // Device found
        }
    }

    // New device, add to stats
    if (device_count < MAX_DEVICES) {
        memcpy(device_stats[device_count].mac, mac_addr, 6);
        device_stats[device_count].rx_bytes = 0;
        device_stats[device_count].tx_bytes = 0;
        return device_count++;
    }

    return -1;  // Device table full
}

// Packet handler function for libpcap
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    struct ether_header *eth_header;
    eth_header = (struct ether_header *) packet;

    // Check if it's IP traffic (EtherType == 0x0800)
    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        // Find or add source and destination devices
        int src_index = find_or_add_device(eth_header->ether_shost);
        int dst_index = find_or_add_device(eth_header->ether_dhost);

        // Update usage statistics for source and destination
        if (src_index >= 0) {
            device_stats[src_index].tx_bytes += pkthdr->len;
        }
        if (dst_index >= 0) {
            device_stats[dst_index].rx_bytes += pkthdr->len;
        }
    }
}

// Function to print device usage
void print_device_usage() {
    printf("\nPer-Device Bandwidth Usage:\n");
    for (int i = 0; i < device_count; i++) {
        printf("Device %02x:%02x:%02x:%02x:%02x:%02x -> Download: %lu bytes, Upload: %lu bytes\n",
               device_stats[i].mac[0], device_stats[i].mac[1], device_stats[i].mac[2],
               device_stats[i].mac[3], device_stats[i].mac[4], device_stats[i].mac[5],
               device_stats[i].rx_bytes, device_stats[i].tx_bytes);
    }
}

int main() {
    struct net_interface old_stat = {0};
    struct net_interface new_stat = {0};
    const char *interface_name = "enp0s3"; 
    
    // Get initial network stats
    get_network_stats(&old_stat, interface_name);

    // Initialize libpcap to capture packets
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_live(interface_name, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", interface_name, errbuf);
        return 1;
    }

    printf("Monitoring interface: %s\n", interface_name);

    while (1) {
        // Capture packets for 1 second
        pcap_dispatch(handle, 10, packet_handler, NULL);

        // Get new network stats
        get_network_stats(&new_stat, interface_name);

        // Calculate and display interface-level bandwidth usage
        calculate_bandwidth_usage(&old_stat, &new_stat, 1.0); // Interval = 1 second

        // Print per-device bandwidth usage
        print_device_usage();

        // Copy new stats to old stats for the next iteration
        old_stat = new_stat;

        sleep(1); // Wait for 1 second before next round
    }

    // Cleanup
    pcap_close(handle);
    return 0;
}