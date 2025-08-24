# Use a lightweight C build environment
FROM debian:bullseye

# Install build tools and libmicrohttpd
RUN apt-get update && apt-get install -y \
    gcc make libmicrohttpd-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy your source code
WORKDIR /app
COPY . .

# Build hello and server2
RUN gcc hello.c -o hello
RUN gcc server2.c -o server2 -lmicrohttpd

# Expose Render's required port
EXPOSE 10000

# Start your server
CMD ["./server2"]
