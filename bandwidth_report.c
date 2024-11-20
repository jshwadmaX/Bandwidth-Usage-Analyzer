#include <pcap.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <time.h>

#define INTERVAL 20 // Report every 20 seconds

u_int total_bytes = 0;
time_t start_time;

// Function to get packet size and timestamp
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    total_bytes += pkthdr->len;

    printf("Time: %s", ctime((const time_t*)&pkthdr->ts.tv_sec));
    printf("Packet Size: %u bytes\n", pkthdr->len);
    printf("----------------------------------------\n");

    time_t current_time = time(NULL);
    
    // Report bandwidth after every INTERVAL seconds
    if (current_time - start_time >= INTERVAL) {
        double bandwidth = total_bytes / (double) INTERVAL;
        printf("\n--- Bandwidth Report ---\n");
        printf("Total Bytes: %u bytes\n", total_bytes);
        printf("Average Bandwidth: %.2f bytes/second\n", bandwidth);
        printf("------------------------\n\n");

        // Reset counters for the next interval
        total_bytes = 0;
        start_time = current_time;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <device>\n", argv[0]);
        return 1;
    }

    char *dev = argv[1];
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;

    // Open the device for live capture
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        printf("Couldn't open device %s: %s\n", dev, errbuf);
        return 2;
    }

    printf("Listening on %s...\n", dev);
    
    // Initialize the start time
    start_time = time(NULL);

    // Start capturing packets
    pcap_loop(handle, 0, packet_handler, NULL);
    
  

    // Close the handle
    pcap_close(handle);
    
    
    
    return 0;
}