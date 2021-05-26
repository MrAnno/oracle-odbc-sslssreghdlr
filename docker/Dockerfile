FROM archlinux:latest

RUN pacman -Syu --noconfirm curl unzip base-devel unixodbc libaio gdb vim

#FROM ubuntu:20.04

#ENV DEBIAN_FRONTEND=noninteractive
# RUN apt-get update && apt-get install -y \
#   curl \
#   unzip \
#   build-essential \
#   pkg-config \
#   unixodbc-dev \
#   libaio1 \
#   gdb \
#   vim \
#   && rm -rf /var/lib/apt/lists/*

RUN curl -LO https://download.oracle.com/otn_software/linux/instantclient/211000/instantclient-basic-linux.x64-21.1.0.0.0.zip \
  && curl -LO https://download.oracle.com/otn_software/linux/instantclient/211000/instantclient-odbc-linux.x64-21.1.0.0.0.zip \
  && mkdir -p /opt/oracle && cd /opt/oracle && unzip /instantclient-basic-linux.x64-21.1.0.0.0.zip && unzip /instantclient-odbc-linux.x64-21.1.0.0.0.zip

ENV LD_LIBRARY_PATH=/opt/oracle/instantclient_21_1:$LD_LIBRARY_PATH
