#!/bin/bash

function run_traffic_monitor() {
    echo "Running Traffic Monitoring..."
    sudo ./traffic_monitor 
    echo "Network Monitoring Completed."
}

function run_bandwidth_track() {
    echo "Running Per Device Tracking..."
    sudo ./bandwidth_track
    echo "Per Device Tracking Completed."
}

function run_bandwidth_report() {
    echo "Running Bandwidth Utilization Reports..."
    sudo ./bandwidth_report enp0s3
    echo "Bandwidth Utilization Reports Completed."
}

function run_bandwidth_alert() {
    echo "Running Alerts and Notifications..."
    
    # Run the bandwidth alert program in the background
    sudo ./bandwidth_alert &
    
    tail -f /var/log/bandwidth_alerts.log

    echo "Alerts and Notifications Completed."
}

function run_protocol_monitor() {
    echo "Running Protocol Monitor..."
    sudo ./protocol_monitor
    echo "Protocol Monitor Completed."
}

function run_network_stats() {
    echo "Running Network Statistics..."
    sudo ./bandwidth_stats
    echo "Network Statistics Completed."
}

function run_bandwidth_cost() {
    echo "Running Bandwidth Cost estimation..."
    sudo ./bandwidth_cost
    echo "Bandwidth Cost Estimation Completed."
}

while true; do
    echo -e "\n"  # Add a newline for better readability
    echo "Starting Bandwidth Usage Analyzer..."
    echo "Please select an option:"
    echo "1) Traffic Monitoring"
    echo "2) Per Device Tracking"
    echo "3) Bandwidth Utilization Reports"
    echo "4) Alerts and Notifications"
    echo "5) Protocol Monitor"
    echo "6) Network Statistics"
    echo "7) Bandwidth Cost Estimation"
    echo "8) Exit"

    read -p "Enter your choice (1-8): " choice

    case $choice in
        1) run_traffic_monitor ;;
        2) run_bandwidth_track ;;
        3) run_bandwidth_report ;;
        4) run_bandwidth_alert ;;
        5) run_protocol_monitor ;;
        6) run_network_stats ;;
        7) run_bandwidth_cost ;;
        8) echo "Exiting..."; break ;;
        *) echo "Invalid option, please try again." ;;
    esac

    echo ""  # Print a blank line for better readability
done

echo "All features executed."