FROM ubuntu:22.04

COPY . .
WORKDIR /simple_network

RUN apt-get update
RUN apt-get install -y cmake
RUN apt-get install -y gcc
RUN apt-get install -y build-essential 
RUN apt-get install -y gdb 
RUN apt-get install -y git
RUN apt-get install -y valgrind