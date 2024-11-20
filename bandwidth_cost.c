#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void get_network_stats(const char* interface, unsigned long long* rx_bytes, unsigned long long* tx_bytes) {
    FILE* file = fopen("/proc/net/dev", "r");
    if (!file) {
        perror("Failed to open /proc/net/dev");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, interface)) {
            // Parse the RX and TX bytes from the line
            sscanf(line + strcspn(line, ":") + 1, "%llu %*s %*s %*s %*s %*s %*s %*s %llu", rx_bytes, tx_bytes);//find line whith : and shift pointer to right(+1) 
            break;//discard %s in middle and just take rx tx values
        }
    }

    fclose(file);
}

int main() {
    const char* interface = "enp0s3"; //set network iterface
    const double cost_per_gb_inr = 10.0; // Cost per GB in INR

    unsigned long long prev_rx_bytes = 0, prev_tx_bytes = 0;
    unsigned long long curr_rx_bytes = 0, curr_tx_bytes = 0;

    // Initial reading of network stats
    get_network_stats(interface, &prev_rx_bytes, &prev_tx_bytes);

    while (1) {
        sleep(100); // Monitor every 100 seconds

        // Get current network stats
        get_network_stats(interface, &curr_rx_bytes, &curr_tx_bytes);

        
        unsigned long long rx_diff = curr_rx_bytes - prev_rx_bytes;
        unsigned long long tx_diff = curr_tx_bytes - prev_tx_bytes;

        // bytes to gigabytes (1 GB = 1024 * 1024 * 1024 bytes)
        double rx_gb = (double)rx_diff / (1024 * 1024 * 1024);
        double tx_gb = (double)tx_diff / (1024 * 1024 * 1024);

        //  total GB used and apply cost estimation
        double total_gb = rx_gb + tx_gb;
        double estimated_cost_inr = total_gb * cost_per_gb_inr;
        

        // Display usage and cost estimation
        printf("Data received: %.4f GB, Data transmitted: %.4f GB\n", rx_gb, tx_gb);
        printf("Total data: %.4f GB, Estimated cost: ₹%.2f\n", total_gb, estimated_cost_inr);

        // Update previous stats
        prev_rx_bytes = curr_rx_bytes;
        prev_tx_bytes = curr_tx_bytes;
    }

    return 0;
}
