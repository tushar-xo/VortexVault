#!/bin/bash

echo "🚀 Starting VortexVault..."
echo ""

# Change to script directory
cd "$(dirname "$0")"

# Check if rust backend is already running
if lsof -Pi :8080 -sTCP:LISTEN -t >/dev/null ; then
    echo "⚠️  Port 8080 already in use. Stopping existing backend..."
    pkill -f vortex-vault-rust
    sleep 2
fi

# Check if frontend is already running
if lsof -Pi :8000 -sTCP:LISTEN -t >/dev/null ; then
    echo "⚠️  Port 8000 already in use. Stopping existing frontend..."
    pkill -f "python3 -m http.server 8000"
    sleep 2
fi

# Start Rust backend
echo "📦 Starting Rust backend on port 8080..."
cd rust
nohup ./target/release/vortex-vault-rust > /tmp/vortex-backend.log 2>&1 &
BACKEND_PID=$!
cd ..

# Wait for backend to start
sleep 3

# Start frontend
echo "🌐 Starting frontend on port 8000..."
cd frontend
nohup python3 -m http.server 8000 > /tmp/vortex-frontend.log 2>&1 &
FRONTEND_PID=$!
cd ..

# Wait a moment
sleep 2

echo ""
echo "✅ VortexVault is running!"
echo ""
echo "📍 Frontend: http://localhost:8000"
echo "📍 Backend:  http://localhost:8080"
echo ""
echo "🔍 Backend PID: $BACKEND_PID"
echo "🔍 Frontend PID: $FRONTEND_PID"
echo ""
echo "📝 Logs:"
echo "   Backend:  tail -f /tmp/vortex-backend.log"
echo "   Frontend: tail -f /tmp/vortex-frontend.log"
echo ""
echo "🛑 To stop: pkill -f vortex-vault-rust && pkill -f 'python3 -m http.server 8000'"
echo ""
echo "🌐 Opening browser..."
sleep 1

# Try to open browser
if command -v open &> /dev/null; then
    open http://localhost:8000
elif command -v xdg-open &> /dev/null; then
    xdg-open http://localhost:8000
else
    echo "Please open http://localhost:8000 in your browser"
fi

echo ""
echo "✨ Ready! Visit http://localhost:8000"
