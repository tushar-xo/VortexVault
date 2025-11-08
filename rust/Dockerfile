FROM rust:1.70

WORKDIR /app

# First copy only the files needed for dependency resolution
COPY Cargo.toml ./

# Create dummy main.rs to satisfy initial build
RUN mkdir src && echo "fn main() {}" > src/main.rs

# Generate lockfile and build dependencies
RUN cargo generate-lockfile && cargo build --release

# Now copy the actual source code
COPY src ./src

# Build the actual application
RUN cargo build --release

EXPOSE 8081

CMD ["./target/release/vortex-vault-rust"]
