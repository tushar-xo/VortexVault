#!/bin/bash

echo "🛑 Stopping VortexVault..."

# Stop Rust backend
echo "Stopping backend..."
pkill -f vortex-vault-rust

# Stop frontend
echo "Stopping frontend..."
pkill -f "python3 -m http.server 8000"

sleep 1

echo "✅ All services stopped!"
