ARG CUDA_VERSION=11.0
FROM nvidia/cuda:${CUDA_VERSION}-devel-ubuntu18.04
LABEL maintainer="Felix Thaler <thaler@cscs.ch>"

RUN apt-get update -qq && \
    apt-get install -qq -y \
    python3-pip && \
    rm -rf /var/lib/apt/lists/*

ENV LC_ALL=C.UTF-8 LANG=C.UTF-8

COPY . /stencil_benchmarks
RUN pip3 install /stencil_benchmarks

RUN /bin/bash -c 'for c in $(compgen -c sbench); do cup=${c^^} && echo "eval \"$""(_${cup//-/_}_COMPLETE=source $c)\""; done >> ~/.bashrc'
