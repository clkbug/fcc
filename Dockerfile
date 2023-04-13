FROM debian:bullseye-slim

ENV DEBIAN_FRONTEND noninteractive
ENV GIT_SSL_NO_VERIFY 1

# python3 is necessary to build gcc
RUN apt-get update && \
    apt-get install tzdata -y && \
    apt-get install --no-install-recommends -y \
    autoconf automake autotools-dev curl \
    libmpc-dev libmpfr-dev libgmp-dev \
    gawk build-essential bison flex texinfo \
    gperf libtool patchutils bc zlib1g-dev libexpat-dev gosu \
    wget ninja-build \
    pkg-config libglib2.0-dev libpixman-1-dev \
    git python3 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ARG WORK_PATH=/work
ARG TMP_PATH=/build-work

RUN \
    mkdir -p ${TMP_PATH} && \
    cd ${TMP_PATH}

RUN \
    cd ${TMP_PATH} && \
    wget --no-check-certificate https://download.qemu.org/qemu-7.1.0.tar.xz && \
    tar xf qemu-7.1.0.tar.xz && \
    cd qemu-7.1.0 && ./configure --prefix=/usr/local/qemu --target-list=riscv32-linux-user --extra-cxxflags="-march=native" && make -j12 install

RUN \
    cd ${TMP_PATH} && \
    git clone https://github.com/riscv-collab/riscv-gnu-toolchain && \
    cd riscv-gnu-toolchain && \
    git checkout 2022.11.23 && \
    ./configure --prefix=/usr/local/gcc/riscv32-unknown-elf --with-arch=rv32i --with-abi=ilp32 && make -j12 && \
    make clean && make distclean && \
    ./configure --prefix=/usr/local/gcc/riscv32im-unknown-elf --with-arch=rv32im --with-abi=ilp32 && make -j12 && \
    ./configure --prefix=/usr/local/gcc/riscv32-linux-gnu --with-arch=rv32g && make linux -j12 && \
    rm ${TMP_PATH} -f -r


ENV PATH $PATH:/usr/local/gcc/riscv32-linux-gnu/bin:/usr/local/gcc/riscv32im-unknown-elf:/usr/local/gcc/riscv32-unknown-elf/bin:/usr/local/qemu/bin


CMD ["/bin/bash"]

# Set a work directory
WORKDIR ${WORK_PATH}

