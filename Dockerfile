FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \   
    cmake \
    g++ \
    wget \
    curl \
    git \
    gnuplot \
    liblapack-dev \
    libpng-dev \
    libjpeg-dev \
    libtiff-dev \
    libfftw3-dev \
    libnetcdf-dev \ 
    libnetcdf-c++4-dev \
    zlib1g-dev \
    libx11-dev \
    libfreetype6-dev \
    libxft-dev \
    libxext-dev \
    libgl1-mesa-dev \
    libfontconfig1-dev \
    libblas-dev

# Clone and build Matplot++
RUN git clone https://github.com/alandefreitas/matplotplusplus.git /matplot && \
cd /matplot && mkdir build && cd build && \
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/app && \
make -j$(nproc) && make install

# Create working directory
WORKDIR /app

# Copy all files into container
COPY . .

# Ensure docker can write png image
RUN chmod 777 /app/assets

RUN cd /app && \
    mkdir build && \
    cd build && \
    cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON && \
    cmake --build . --verbose

# Set default command
EXPOSE 18080

#run app
CMD ["./bin/main"]