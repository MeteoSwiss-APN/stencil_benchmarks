FROM ubuntu:20.04
LABEL maintainer="Felix Thaler <thaler@cscs.ch>"

RUN apt-get update -qq && \
    apt-get install -qq -y \
    gcc \
    clang \
    python3-pip && \
    rm -rf /var/lib/apt/lists/*

COPY . /stencil_benchmarks
RUN pip3 install /stencil_benchmarks

RUN /bin/bash -c 'for c in $(compgen -c sbench); do cup=${c^^} && echo "eval \"$""(_${cup//-/_}_COMPLETE=source $c)\""; done >> ~/.bashrc'
