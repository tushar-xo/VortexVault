VORTEX VAULT 

bash initialize.sh

VORTEX-VAULT/
├── cpp/                    # C++ backend
│   ├── src/
│   │   ├── main.cpp      # Crow API, k-d tree
│   │   ├── KDTreeVectorDB.h/cpp
│   │   ├── embed.cpp     # bert.cpp for embeddings
│   ├── Dockerfile
│   ├── CMakeLists.txt
├── rust/                   # Rust backend
│   ├── src/
│   │   ├── main.rs       # Actix-Web API, k-d tree
│   │   ├── kdtree.rs
│   │   ├── embed.rs      # sentence-transformers
│   ├── Cargo.toml
│   ├── Dockerfile
├── frontend/               # HTML/JS frontend
│   ├── index.html
│   ├── scripts.js        # D3.js, Chart.js for visualizations
│   ├── styles.css
├── data/                   # Mounted volume for persistence
│   ├── vecdb.bin         # C++ storage
│   ├── vecdb.rsbin       # Rust storage
├── docker-compose.yml      # Orchestrates C++/Rust backends, frontend, MySQL
├── README.md


# TO RUN
Execute bash initialize.sh to set up directories

Run docker-compose up --build to start all services

Access frontend at http://localhost, C++ API at 8080, Rust at 8081