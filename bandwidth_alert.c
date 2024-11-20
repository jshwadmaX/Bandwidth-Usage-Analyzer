#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

// Define a lower threshold for bandwidth usage in bytes (e.g., 100MB)
#define THRESHOLD_BYTES (10 * 1024 * 1024)  // 10MB

long get_network_usage(const char* interface) {
    FILE *f = fopen("/proc/net/dev", "r");
    if (!f) {
        perror("Failed to open /proc/net/dev");
        return -1;
    }

    char line[512];
    long rx_bytes = 0, tx_bytes = 0;
    while (fgets(line, sizeof(line), f)) {
        // Find the line for the given network interface
        if (strstr(line, interface)) {
            // Parse the line to extract received and transmitted bytes
            sscanf(line, "%*s %ld %*s %*s %*s %*s %*s %*s %*s %ld", &rx_bytes, &tx_bytes);
            break;
        }
    }

    fclose(f);
    return rx_bytes + tx_bytes;
}

void send_alert(const char* message) {
    // Log the alert to a file
    FILE *log = fopen("/var/log/bandwidth_alerts.log", "a");
    if (log) {
        fprintf(log, "ALERT: %s\n", message);
        fclose(log);
    } else {
        perror("Failed to open log file");
    }

    // Also print to console
    printf("ALERT: %s\n", message);
}

int main() {
    const char* interface = "enp0s3";  // Network interface to monitor
    long threshold = THRESHOLD_BYTES;
    long prev_usage = get_network_usage(interface);

    if (prev_usage == -1) {
        fprintf(stderr, "Error: Could not get network usage for %s\n", interface);
        return 1;
    }

    printf("Monitoring bandwidth usage for interface: %s\n", interface);

    // Monitor the network usage over time
    while (1) {
        sleep(60);  // Check every 60 seconds

        long curr_usage = get_network_usage(interface);
        if (curr_usage == -1) {
            fprintf(stderr, "Error: Could not get network usage for %s\n", interface);
            continue;
        }

        long usage_diff = curr_usage - prev_usage;

        // Check if the bandwidth usage exceeds the threshold
        if (usage_diff > threshold) {
            char alert_message[128];
            snprintf(alert_message, sizeof(alert_message), 
                     "High bandwidth usage detected on %s: %ld bytes in last interval",
                     interface, usage_diff);
            send_alert(alert_message);
        }

        // Update previous usage
        prev_usage = curr_usage;
    }

    return 0;
}
