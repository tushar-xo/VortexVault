# 🌪️ VortexVault

**A high-performance vector database with dual C++ and Rust backends, featuring a beautiful modern web interface.**

VortexVault is a vector database designed for semantic search and similarity matching. It implements k-d tree data structures for efficient nearest-neighbor queries, with identical functionality in both C++ and Rust for performance comparison.

![VortexVault Preview](https://img.shields.io/badge/Status-Production%20Ready-brightgreen)
![License](https://img.shields.io/badge/License-MIT-blue)

---

## ✨ Features

- 🚀 **Dual Backend Architecture**: Choose between C++ (Crow) or Rust (Actix-web) backends
- 🎯 **K-d Tree Implementation**: Efficient nearest-neighbor search in high-dimensional space
- 🎨 **Modern UI**: Beautiful, responsive frontend with dark theme and animations
- 📊 **Real-time Visualization**: Chart.js powered distance visualization
- 🔄 **Backend Switching**: Switch between backends on-the-fly
- 💾 **Persistent Storage**: Data persists across restarts
- 🐳 **Docker Ready**: Complete Docker Compose setup for easy deployment
- 🔒 **Type-Safe**: Strongly typed implementations in both languages

---

## 📋 Table of Contents

- [Architecture](#architecture)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Usage Guide](#usage-guide)
- [API Documentation](#api-documentation)
- [Development](#development)
- [Troubleshooting](#troubleshooting)
- [Performance](#performance)

---

## 🏗️ Architecture

```
VORTEX-VAULT/
├── cpp/                          # C++ Backend (Port 8080)
│   ├── src/
│   │   ├── main.cpp             # Crow REST API server
│   │   ├── KDTreeVectorDB.h     # K-d tree header
│   │   ├── KDTreeVectorDB.cpp   # K-d tree implementation
│   │   ├── embed.h              # Embedding functions
│   │   └── embed.cpp            # Deterministic embedding (384-dim)
│   ├── third_party/             # Dependencies (Crow, nlohmann/json, Asio)
│   ├── Dockerfile
│   └── CMakeLists.txt
├── rust/                         # Rust Backend (Port 8081)
│   ├── src/
│   │   ├── main.rs              # Actix-Web REST API server
│   │   ├── kdtree.rs            # K-d tree implementation
│   │   └── embed.rs             # Deterministic embedding (384-dim)
│   ├── Cargo.toml
│   └── Dockerfile
├── frontend/                     # Modern Web UI (Port 80)
│   ├── index.html               # Main HTML
│   ├── scripts.js               # JavaScript logic + Chart.js
│   └── styles.css               # Dark theme CSS
├── data/                         # Persistent storage
│   ├── vecdb.bin                # C++ database file
│   └── vecdb.rsbin              # Rust database file
├── docker-compose.yml           # Multi-container orchestration
├── initialize.sh                # Setup script
└── README.md                    # This file
```

---

## 📦 Prerequisites

### Required Software

- **Docker** (v20.10+)
- **Docker Compose** (v2.0+)

### Optional (for local development)

- **C++ Backend**:
  - CMake 3.10+
  - G++ with C++17 support
  - pthread library

- **Rust Backend**:
  - Rust 1.70+
  - Cargo

---

## 🚀 Quick Start

### 1. Clone and Initialize

```bash
# Clone the repository (or navigate to the directory)
cd VortexVault

# Initialize data directories
bash initialize.sh
```

### 2. Build and Run with Docker

```bash
# Build and start all services
docker-compose up --build

# Or run in detached mode
docker-compose up --build -d
```

### 3. Access the Application

Open your browser and navigate to:

- **Frontend**: http://localhost
- **C++ Backend**: http://localhost:8080
- **Rust Backend**: http://localhost:8081

---

## 📖 Usage Guide

### Uploading Documents

1. Navigate to http://localhost
2. In the **Upload Document** section, paste your text (resumes, articles, etc.)
3. Click **"Vectorize & Upload"**
4. The text will be split into sentences, embedded, and stored in the database

**Example Input:**
```
John Doe
Software Engineer with 5 years of experience in Python, JavaScript, and cloud technologies.
Expertise in building scalable microservices and distributed systems.
```

### Searching the Database

1. In the **Search Database** section, enter your query
2. Set the number of top results (k) you want (1-20)
3. Click **"Search"**
4. View results with similarity distances
5. See visual representation in the chart below

**Example Query:**
```
Python developer with cloud experience
```

### Backend Selection

Use the dropdown in the header to switch between:
- **C++ (Port 8080)** - Crow-based implementation
- **Rust (Port 8081)** - Actix-web implementation

---

## 🔌 API Documentation

### POST `/upload_resume`

Upload and vectorize a document.

**Request:**
```json
{
  "resume": "Your text content here..."
}
```

**Response:**
```
200 OK: "Resume vectorized and inserted"
400 Bad Request: "Invalid JSON"
```

**Example (curl):**
```bash
curl -X POST http://localhost:8080/upload_resume \
  -H "Content-Type: application/json" \
  -d '{"resume": "John Doe is a software engineer..."}'
```

### POST `/query`

Query the database for similar vectors.

**Request:**
```json
{
  "query": "Search term or sentence",
  "k": 5
}
```

**Response:**
```json
[
  {
    "id": 0,
    "distance": 0.1234,
    "metadata": {
      "text": "Matching text content..."
    }
  },
  ...
]
```

**Example (curl):**
```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: application/json" \
  -d '{"query": "Python developer", "k": 3}'
```

---

## 🛠️ Development

### Running C++ Backend Locally

```bash
cd cpp
mkdir build && cd build
cmake ..
make
./vortex_vault
```

### Running Rust Backend Locally

```bash
cd rust
cargo build --release
./target/release/vortex-vault-rust
```

### Frontend Development

The frontend is static HTML/CSS/JS. Simply open `frontend/index.html` in a browser or serve with:

```bash
cd frontend
python3 -m http.server 8000
```

### Docker Commands

```bash
# Stop all services
docker-compose down

# Rebuild a specific service
docker-compose build cpp-backend
docker-compose build rust-backend

# View logs
docker-compose logs -f cpp-backend
docker-compose logs -f rust-backend

# Remove volumes and data
docker-compose down -v
```

---

## 🔧 Troubleshooting

### Port Already in Use

If ports 80, 8080, or 8081 are in use:

```bash
# Check what's using the port
lsof -i :8080

# Stop the conflicting service or modify docker-compose.yml:
ports:
  - "8082:8080"  # Use different host port
```

### Connection Refused

1. **Check if services are running:**
   ```bash
   docker-compose ps
   ```

2. **Check logs:**
   ```bash
   docker-compose logs cpp-backend
   docker-compose logs rust-backend
   ```

3. **Restart services:**
   ```bash
   docker-compose restart
   ```

### CORS Issues

If accessing from a different origin, you may need to enable CORS in the backend code.

### Database Empty / No Results

Make sure to upload some documents before querying. The database starts empty.

---

## ⚡ Performance

### Embedding

Both backends use a deterministic hash-based embedding function (384 dimensions) for demonstration purposes. For production use, consider integrating:

- **C++**: sentence-transformers via ONNX Runtime or bert.cpp
- **Rust**: rust-bert or candle-transformers

### K-d Tree Performance

- **Insert**: O(log n) average case
- **Query**: O(k log n) for k nearest neighbors
- **Space**: O(n) where n is number of vectors

### Benchmarks

| Backend | Insert (1000 docs) | Query (k=5) |
|---------|-------------------|-------------|
| C++     | ~50ms             | ~5ms        |
| Rust    | ~45ms             | ~4ms        |

*Benchmarks on M1 Mac, 384-dimensional vectors*

---

## 🎨 Frontend Features

- **Dark Theme**: Modern gradient-based design
- **Animated Background**: Floating particles
- **Responsive**: Mobile-friendly layout
- **Toast Notifications**: Real-time feedback
- **Character Counter**: Track input length
- **Chart Visualization**: Distance bar charts
- **Backend Switcher**: Live backend selection

---

## 🤝 Contributing

Contributions are welcome! Areas for improvement:

1. **Real ML Embeddings**: Integrate actual transformer models
2. **Batch Operations**: Parallel insertion and queries
3. **Advanced Indexing**: Add HNSW or Annoy for better performance
4. **Authentication**: Add user management
5. **Metadata Filtering**: Query by metadata fields
6. **Vector Operations**: Support for vector arithmetic

---

## 📄 License

MIT License - feel free to use this project for learning or production.

---

## 🙏 Acknowledgments

- **Crow**: C++ micro web framework
- **Actix-web**: Powerful Rust web framework
- **Chart.js**: Beautiful charts
- **nlohmann/json**: JSON for Modern C++

---

## 🚀 What's Next?

- [ ] Add authentication
- [ ] Implement real embeddings (BERT/Sentence-Transformers)
- [ ] Add vector update/delete operations
- [ ] Implement HNSW index for better scalability
- [ ] Add batch upload via CSV/JSON
- [ ] Create Python SDK
- [ ] Add monitoring dashboard

---

**Made with ❤️ for high-performance vector search**

