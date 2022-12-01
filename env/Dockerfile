#FROM ubuntu:20.04
FROM debian:bullseye-slim

ENV DEBIAN_FRONTEND noninteractive
ENV GIT_SSL_NO_VERIFY 1

# python3 is necessary to build gcc
RUN apt-get update && \
    apt-get install tzdata -y && \
    apt-get install -y \
    autoconf automake autotools-dev curl \
    libmpc-dev libmpfr-dev libgmp-dev \
    gawk build-essential bison flex texinfo \
    gperf libtool patchutils bc zlib1g-dev libexpat-dev gosu \
    git python3 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ARG WORK_PATH=/work
ARG TMP_PATH=/build-work
ARG SIM_ENV_GCC_PREFIX_BASE=/usr/local

RUN mkdir -p ${TMP_PATH} ${WORK_PATH} && \
    cd ${TMP_PATH} && \
    git clone https://github.com/riscv-collab/riscv-gnu-toolchain && \
    cd riscv-gnu-toolchain && \
    mkdir -p ${SIM_ENV_GCC_PREFIX_BASE} && \
    git checkout ${RISCV32_LINUX_GCC_TAG} -f && \
    git submodule update --init --recursive -f && \
    CFLAGS="-O2 -static" ./configure --prefix=${SIM_ENV_GCC_PREFIX_BASE} --with-arch=rv32g && \
    make -j10


ENTRYPOINT [ "/bin/bash" ]

