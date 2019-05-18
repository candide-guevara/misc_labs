#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include <common.h>
#include <logger.h>
#include <sockaddr_util.h>

void extra_sockopt(int socket_fd) {
  LOG_INFO("OPTION bind socket=%d to wlp3s0", socket_fd);
  devbind(socket_fd, "wlp3s0");
}

void* vanilla_tcp_clt(void * to_addr) {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  DIE_IF_EQ(socket_fd, -1);
  struct sockaddr* srv_addr = create_ip4_addr(to_addr, 8080);

  //extra_sockopt(socket_fd);
  LOG_INFO("CLIENT Connecting to type=%d, port=%d, addr=%s", 
    get4family(srv_addr), get4port(srv_addr), get4addr(srv_addr) );
  DIE_IF_ERR( connect(socket_fd, srv_addr, IPV4_LEN) );
  struct sockaddr* client_addr = get64sockname(socket_fd);

  LOG_INFO("CLIENT Connected (type=%d, port=%d, addr=%s) <=> (type=%d, port=%d, addr=%s)", 
    get4family(client_addr), get4port(client_addr), get4addr(client_addr),
    get4family(srv_addr), get4port(srv_addr), get4addr(srv_addr) );

  DIE_IF_ERR( shutdown(socket_fd, SHUT_RDWR) );
  DIE_IF_ERR( close(socket_fd) );
  return NULL;
}

void* vanilla_tcp_clt6(void * to_addr) {
  int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
  DIE_IF_EQ(socket_fd, -1);
  struct sockaddr* srv_addr = create_ip6_addr(to_addr, 8080);

  //extra_sockopt(socket_fd);
  LOG_INFO("CLIENT Connecting to type=%d, port=%d, addr=%s", 
    get6family(srv_addr), get6port(srv_addr), get6addr(srv_addr) );
  DIE_IF_ERR( connect(socket_fd, srv_addr, IPV6_LEN) );
  struct sockaddr* client_addr = get64sockname(socket_fd);

  LOG_INFO("CLIENT Connected (type=%d, port=%d, addr=%s) <=> (type=%d, port=%d, addr=%s)", 
    get6family(client_addr), get6port(client_addr), get6addr(client_addr),
    get6family(srv_addr), get6port(srv_addr), get6addr(srv_addr) );

  DIE_IF_ERR( shutdown(socket_fd, SHUT_RDWR) );
  DIE_IF_ERR( close(socket_fd) );
  return NULL;
}

void* vanilla_tcp_srv(const char* srv_addr, char* to_addr, void *client_routine, void (*custom_sockopt)(int)) {
  const int max_pending_cnx = 1;
  struct sockaddr* client_addr = create_ip4_addr(NULL, 0);
  uint32_t client_len = MAX_ADDR_LEN;

  struct sockaddr* bind_to = create_ip4_addr(srv_addr, 8080);

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  DIE_IF_EQ(socket_fd, -1);
  if (custom_sockopt) custom_sockopt(socket_fd);
  DIE_IF_ERR( bind(socket_fd, bind_to, IPV4_LEN) );
  DIE_IF_ERR( listen(socket_fd, max_pending_cnx) );

  pthread_t client_thread;
  DIE_IF_ERR( pthread_create(&client_thread, NULL, client_routine, to_addr) ); 

  LOG_INFO("SERVER Accepting at type=%d, port=%d, addr=%s", 
    get4family(bind_to), 
    get4port(bind_to),
    get4addr(bind_to) );

  int cnx_fd = accept(socket_fd, client_addr, &client_len);
  DIE_IF_EQ(cnx_fd, -1);

  LOG_INFO("SERVER Connected to type=%d, port=%d, addr=%s", 
    get4family(client_addr), 
    get4port(client_addr),
    get4addr(client_addr) );

  DIE_IF_ERR( pthread_join(client_thread, NULL) );

  DIE_IF_ERR( shutdown(cnx_fd, SHUT_RDWR) );
  DIE_IF_ERR( close(cnx_fd) );
  DIE_IF_ERR( close(socket_fd) );
  return NULL;
}

void* vanilla_tcp_srv6(const char* srv_addr, char* to_addr, void *client_routine, void (*custom_sockopt)(int)) {
  const int max_pending_cnx = 1;
  struct sockaddr* client_addr = create_ip6_addr(NULL, 0);
  uint32_t client_len = MAX_ADDR_LEN;

  struct sockaddr* bind_to = create_ip6_addr(srv_addr, 8080);

  int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
  DIE_IF_EQ(socket_fd, -1);
  if (custom_sockopt) custom_sockopt(socket_fd);
  DIE_IF_ERR( bind(socket_fd, bind_to, IPV6_LEN) );
  DIE_IF_ERR( listen(socket_fd, max_pending_cnx) );

  pthread_t client_thread;
  DIE_IF_ERR( pthread_create(&client_thread, NULL, client_routine, to_addr) ); 

  LOG_INFO("SERVER Accepting at type=%d, port=%d, addr=%s", 
    get6family(bind_to), 
    get6port(bind_to),
    get6addr(bind_to) );

  int cnx_fd = accept(socket_fd, client_addr, &client_len);
  DIE_IF_EQ(cnx_fd, -1);

  LOG_INFO("SERVER Connected to type=%d, port=%d, addr=%s", 
    get6family(client_addr), 
    get6port(client_addr),
    get6addr(client_addr) );

  DIE_IF_ERR( pthread_join(client_thread, NULL) );

  DIE_IF_ERR( shutdown(cnx_fd, SHUT_RDWR) );
  DIE_IF_ERR( close(cnx_fd) );
  DIE_IF_ERR( close(socket_fd) );
  return NULL;
}

void try_6_4_combi(const char* srv_addr4, const char* clt_addr4, const char* srv_addr6, const char* clt_addr6, void (*custom_sockopt)(int)) {
  // srv any_ipv4 / clt ipv4 => OK
  // srv ipv4 / clt ipv4 => OK
  // srv ipv4_devbind / clt ipv4 => OK
  // srv ipv4_devbind / clt ipv4_devbind => OK
  // srv ipv4_loc_devbind / clt ipv4 => KO
  LOG_INFO("srv4/clt4");
  vanilla_tcp_srv(srv_addr4, (char*)clt_addr4, vanilla_tcp_clt, custom_sockopt);

  // srv any_ipv6 / clt ipv6 => OK
  // srv ipv6 / clt ipv6 => OK
  // srv link ipv6 / clt link ipv6 => KO (Invalid argument) / OK if you specify the interface
  // srv ipv6_devbind / clt ipv6 => OK
  // srv ipv6_devbind / clt ipv6_devbind => OK
  // srv ipv6_loc_devbind / clt ipv6 => KO
  LOG_INFO("srv6/clt6");
  vanilla_tcp_srv6(srv_addr6, (char*)clt_addr6, vanilla_tcp_clt6, custom_sockopt);

  // srv any_ipv6 / clt ipv4 => OK
  // srv ipv6 / clt ipv4 => KO (connection refused)
  // srv ipv4_devbind / clt ipv4 => NA
  // srv ipv4_loc_devbind / clt ipv4 => NA
  LOG_INFO("srv6/clt4");
  vanilla_tcp_srv6(srv_addr6, (char*)clt_addr4, vanilla_tcp_clt, custom_sockopt);

  // srv any_ipv4 / clt ipv6 => KO (connection refused)
  // srv ipv4 / clt ipv6 => KO (connection refused)
  // srv ipv4_devbind / clt ipv4 => NA
  // srv ipv4_loc_devbind / clt ipv4 => NA
  //LOG_INFO("srv4/clt6");
  //vanilla_tcp_srv(srv_addr4, (char*)clt_addr6, vanilla_tcp_clt6, custom_sockopt);
}

int main (void) {
  LOG_WARN("Starting ...");

  //try_6_4_combi("0.0.0.0", "192.168.43.139", "::", "2a00:1450:4001:815::200e", NULL);
  //try_6_4_combi("192.168.43.139", "192.168.43.139", "2a00:1450:4001:815::200e", "2a00:1450:4001:815::200e", NULL);
  //try_6_4_combi("127.0.0.1", "127.0.0.1", "::1", "::1", NULL);
  // sudo ip addr add 2a00:1450:4001:815::200e dev <DEVNAME>
  // sudo ip addr add 169.254.12.12/16 dev <DEVNAME> scope link
  try_6_4_combi("169.254.12.12", "169.254.12.12", "fe80::552e:1c80:8c28:e072", "fe80::552e:1c80:8c28:e072", NULL);

  LOG_INFO("All done !!");
  return 0;
}

