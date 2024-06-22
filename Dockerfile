FROM ubuntu:22.04

COPY . .
WORKDIR /cpp_sandbox

RUN apt-get update
RUN apt-get install -y cmake
RUN apt-get install -y gcc
RUN apt-get install -y build-essential 
RUN apt-get install -y gdb 
RUN apt-get install -y git 
RUN apt-get install -y libboost-all-dev

RUN mkdir cmake
RUN apt-get install -y wget
RUN wget -P ./cmake -O ./cmake/CPM.cmake https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
