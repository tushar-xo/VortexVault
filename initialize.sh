#!/bin/bash

echo "Initializing VortexVault..."

# Create necessary directories
mkdir -p data
mkdir -p cpp/src
mkdir -p rust/src
mkdir -p frontend

# Download models or dependencies if needed
# For bert.cpp: git clone https://github.com/skeskinen/bert.cpp && cd bert.cpp && mkdir build && cd build && cmake .. && make
# Adjust paths in code

echo "Setup complete. Run 'docker-compose up --build' to start services."
