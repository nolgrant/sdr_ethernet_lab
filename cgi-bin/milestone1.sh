#!/bin/bash

# --- Read in the config file ---

CONFIG_FILE="./config.txt"

# Check if the config file exists
if [[ ! -f "$CONFIG_FILE" ]]; then
    echo "Error: Configuration file not found at $CONFIG_FILE"
    exit 1
fi

# Extract IP and Port using awk
# awk searches for lines starting with SERVER_IP or SERVER_PORT
# it splits the line by the '=' delimiter, and prints the second field (the value)
SERVER_IP=$(awk -F'=' '/^SERVER_IP/ {print $2}' "$CONFIG_FILE" | tr -d '[:space:]')
SERVER_PORT=$(awk -F'=' '/^SERVER_PORT/ {print $2}' "$CONFIG_FILE" | tr -d '[:space:]')

# Verify extraction was successful (optional but recommended)
if [[ -z "$SERVER_IP" || -z "$SERVER_PORT" ]]; then
    echo "Error: Could not extract both IP and Port from $CONFIG_FILE"
    exit 1
fi

# --- Execute Milestone 1 ---
./milestone1 "$SERVER_IP" "$SERVER_PORT" 1000