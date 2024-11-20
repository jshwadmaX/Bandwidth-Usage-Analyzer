#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux/if_ether.h> // For ETH_P_IP, ETH_P_ARP, and struct ethhdr

void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    //print length
    printf("Packet Length: %d\n", header->len);

    // Check the Ethernet header
    struct ethhdr *eth = (struct ethhdr *) packet;
    printf("Source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth->h_source[0], eth->h_source[1], eth->h_source[2],
           eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    printf("Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
           eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);

    // Check the EtherType to determine the protocol
    switch(ntohs(eth->h_proto)) {
        case ETH_P_IP: 
            printf("This is an IPv4 packet.\n");
            break;
        case ETH_P_ARP: 
            printf("This is an ARP packet.\n");
            break;
        case ETH_P_IPV6: 
            printf("This is an IPv6 packet.\n");
            break;
        default:
            printf("Unknown protocol: 0x%X\n", ntohs(eth->h_proto));
            break;
    }
    printf("\n");
}

int main() {
    pcap_if_t *all_devices, *device;  //structure used to show network interface
    char errbuf[PCAP_ERRBUF_SIZE];   //bufferr to store errors size if STD Set in libcap library 256bytes 

    
    if (pcap_findalldevs(&all_devices, errbuf) == -1) {
        fprintf(stderr, "Error finding devices: %s\n", errbuf);   //retrives a list of all available network devices
        return 1;
    }

    // Print available devices and select the first one
    int i = 0;
    for (device = all_devices; device != NULL; device = device->next) {
        printf("%d. %s - %s\n", ++i, device->name, device->description ? device->description : "No description available");
    }

    if (i == 0) {
        printf("No interfaces found! Make sure you have the proper privileges.\n");
        return 1;
    }

    // Choose the first device from the list
    device = all_devices;  
    printf("Using device: %s\n", device->name);

    
    pcap_t *handle = pcap_open_live(device->name, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Could not open device %s: %s\n", device->name, errbuf);//immediately display error
        return 1;
    }

    // capturing packets
    pcap_loop(handle, 0, packet_handler, NULL);

    // Cleanup
    pcap_freealldevs(all_devices);  // Free the device list
    pcap_close(handle);  // Close the capture handle

    return 0;
}
