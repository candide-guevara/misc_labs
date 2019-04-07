#pragma once

#include <arpa/inet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include <common.h>
#include <logger.h>

#define IPV4_LEN sizeof(struct sockaddr_in)
#define IPV6_LEN sizeof(struct sockaddr_in6)
#define MAX_ADDR_LEN 64

struct sockaddr* create_ip4_addr(const char* str_addr, uint16_t port) {
  // ugly but sets the memory to zero
  LOG_INFO("create addr4 %s:%d", str_addr, port);
  struct sockaddr_in* addr = calloc(1, IPV4_LEN);
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  if (str_addr)
    DIE_IF_NEQ(inet_pton(addr->sin_family, str_addr, &addr->sin_addr), 1)
  return (struct sockaddr*)addr;
}

struct sockaddr* create_ip6_addr(const char* str_addr, uint16_t port) {
  // ugly but sets the memory to zero
  LOG_INFO("create addr6 %s:%d", str_addr, port);
  struct sockaddr_in6* addr = calloc(1, IPV6_LEN);
  addr->sin6_family = AF_INET6;
  addr->sin6_port = htons(port);
  if (str_addr)
    DIE_IF_NEQ(inet_pton(addr->sin6_family, str_addr, &(addr->sin6_addr)), 1)
  return (struct sockaddr*)addr;
}

void devbind(int socket_fd, const char* devname) {
  char* devname_ = calloc(IFNAMSIZ, 1);
  strncpy(devname_, devname, strlen(devname));
  LOG_INFO("[REQUIRES root] binding %d to %s", socket_fd, devname_);
  DIE_IF_ERR( setsockopt(socket_fd, SOL_SOCKET, SO_BINDTODEVICE, devname_, strlen(devname_)) );
}

struct sockaddr* get64sockname(int socket_fd) {
  struct sockaddr* addr = calloc(1, MAX_ADDR_LEN);
  uint32_t client_len = MAX_ADDR_LEN;
  DIE_IF_ERR( getsockname(socket_fd, addr, &client_len) );
  return addr;
}

uint16_t get4port(struct sockaddr* addr) {
  struct sockaddr_in* addr4 = (struct sockaddr_in*) addr;
  return ntohs(addr4->sin_port);
}

uint16_t get6port(struct sockaddr* addr) {
  struct sockaddr_in6* addr6 = (struct sockaddr_in6*) addr;
  return ntohs(addr6->sin6_port);
}

uint32_t get4family(struct sockaddr* addr) {
  struct sockaddr_in* addr4 = (struct sockaddr_in*) addr;
  return addr4->sin_family;
}

uint32_t get6family(struct sockaddr* addr) {
  struct sockaddr_in6* addr6 = (struct sockaddr_in6*) addr;
  return addr6->sin6_family;
}

const char* get4addr(struct sockaddr* addr) {
  char* str_addr = calloc(MAX_ADDR_LEN, 1);
  struct sockaddr_in* addr4 = (struct sockaddr_in*) addr;
  DIE_IF_EQ( inet_ntop(AF_INET, &(addr4->sin_addr), str_addr, MAX_ADDR_LEN), 0 );
  return str_addr;
}

const char* get6addr(struct sockaddr* addr) {
  char* str_addr = calloc(MAX_ADDR_LEN, 1);
  struct sockaddr_in6* addr6 = (struct sockaddr_in6*) addr;
  DIE_IF_EQ( inet_ntop(AF_INET6, &(addr6->sin6_addr), str_addr, MAX_ADDR_LEN), 0 );
  return str_addr;
}

