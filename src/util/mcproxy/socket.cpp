
#include "include.h"
#include "../network.h"

/*
** SOCKET.C
**
** Written by Steven Grimm (koreth@ebay.sun.com) on 11-26-87
** Please distribute widely, but leave my name here.
**
** Various black-box routines for socket manipulation, so I don't have to
** remember all the structure elements.
** Of course, I still have to remember how to call these routines.
*/

/*
** newsocket()
**
** Creates an Internet stream socket.
**
** Output: file descriptor of socket, or a negative error
*/
int newsocket(void)
{
  return socket(AF_INET, SOCK_STREAM, 0);
}

/*
** serversock()
**
** Creates an internet socket, binds it to an address, and prepares it for
** subsequent accept() calls by calling listen().
**
** Input: port number desired, or 0 for a random one
** Output: file descriptor of socket, or a negative error
*/
int serversock(int port)
{
  int  one = 1;
  int  sock, x;
  struct  sockaddr_in server;
  int  sendBufSize = 4194304;
  int  recvBufSize = 1048576;

  sock = newsocket();
  if (sock < 0)
    return -errno;

  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)))
    HPHP::Logger::Error("setsockopt");

  if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvBufSize, sizeof(recvBufSize)))
    HPHP::Logger::Error("setsockopt : SO_RCVBUF");

  if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendBufSize, sizeof(sendBufSize)))
    HPHP::Logger::Error("setsockopt : SO_SNDBUF");

  x = bind(sock, (struct sockaddr *)&server, sizeof(server));
  if (x < 0)
  {
    close(sock);
    return -errno;
  }

  listen(sock, 500);

  return sock;
}

/*
** portnum()
**
** Returns the internet port number for a socket.
**
** Input: file descriptor of socket
** Output: inet port number
*/
int portnum(int fd)
{
  int  err;
  socklen_t length;
  struct  sockaddr_in address;

  length = sizeof(address);
  err = getsockname(fd, (struct sockaddr *)&address, &length);
  if (err < 0)
    return -errno;

  return ntohs(address.sin_port);
}

/*
** host_to_addr()
**
** Returns the IP address of a host in human-readable dotted-quad format.
**
** Input: hostname to look up
**    buffer for IP address
*/
void host_to_addr(const char *hostname, char *buf)
{
  if (isdigit(hostname[0]))
    strcpy(buf, hostname);
  else
  {
    HPHP::Util::HostEnt result;
    if (HPHP::Util::safe_gethostbyname(hostname, result)) {
      struct in_addr addr;
      memcpy(&addr, result.hostbuf.h_addr, result.hostbuf.h_length);
      std::string s = HPHP::Util::safe_inet_ntoa(addr);
      strcpy(buf, s.c_str());
    } else {
      *buf = '\0';
    }
  }
}

/*
** clientsock()
**
** Returns a connected client socket.
**
** Input: host name and port number to connect to
** Output: file descriptor of CONNECTED socket, or a negative error
** (-9999) if the hostname was bad.
*/
int clientsock(char *host, int port)
{
  int  sock;
  struct  sockaddr_in server;

  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  if (isdigit(host[0]))
    server.sin_addr.s_addr = inet_addr(host);
  else
  {
    HPHP::Util::HostEnt result;
    if (!HPHP::Util::safe_gethostbyname(host, result)) {
      return -9999;
    }
    bcopy(result.hostbuf.h_addr, &server.sin_addr, result.hostbuf.h_length);
  }

  sock = newsocket();
  if (sock < 0)
    return -errno;

  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    close(sock);
    return -errno;
  }

  return sock;
}

/*
** clientsock_nb()
**
** Returns a nonblocking client socket with a pending connection attempt.
**
** Input: host name to connect to
**        port number to connect to
**        buffer for file descriptor
** Output: <0 if the connection attempt failed
**         0  if the connection is fully established (e.g. because it's a
**            connection to the local host)
**         >0 if the connection is pending
*/
int clientsock_nb(char *host, int port, int *sock)
{
  struct  sockaddr_in server;

  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  if (isdigit(host[0]))
    server.sin_addr.s_addr = inet_addr(host);
  else
  {
    HPHP::Util::HostEnt result;
    if (!HPHP::Util::safe_gethostbyname(host, result)) {
      return -9999;
    }
    bcopy(result.hostbuf.h_addr, &server.sin_addr, result.hostbuf.h_length);
  }

  *sock = newsocket();
  if (*sock < 0)
    return -1;

  if (fcntl(*sock, F_SETFL, O_NONBLOCK))
    return -1;

  if (connect(*sock, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    if (errno != EINPROGRESS)
    {
      close(*sock);
      return -errno;
    }
  }
  else
    return 0;

  return 1;
}

/*
** waitread()
**
** Wait for data on a file descriptor for a little while.
**
** Input: file descriptor to watch
**    how long to wait, in seconds, before returning
** Output: 1 if data was available
**     0 if the timer expired or a signal occurred.
*/
int waitread(int fd, int time)
{
  fd_set readbits;
  struct timeval timer;

  timerclear(&timer);
  timer.tv_sec = time;
  FD_ZERO(&readbits);
  FD_SET(fd, &readbits);

  select(fd+1, &readbits, NULL, NULL, &timer);
  if (FD_ISSET(fd, &readbits))
    return 1;
  return 0;
}

/*
** readable()
**
** Poll a socket for pending input.  Returns immediately.  This is a front-end
** to waitread() below.
**
** Input: file descriptor to poll
** Output: 1 if data is available for reading
*/
int readable(int fd)
{
  return(waitread(fd, 0));
}

/*
** nodelay()
**
** Set or clear the TCP_NODELAY option on a file descriptor.
**
** Input: file descriptor to modify
**        whether NODELAY is desired
*/
int set_nodelay(int fd, int want_nodelay)
{
  return(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&want_nodelay, sizeof(int)));
}

/*
 * Determines the local network address.
 *
 * Input:  buffer for address
 * Output: 0 on success, -1 if address can't be determined
 */
int determine_local_address(struct in_addr *addrbuf)
{
  int sock;
  struct sockaddr_in addr;
  unsigned int len = sizeof(addr);

  /*
   * Determine local network address by opening a socket and trying
   * to establish a connection somewhere.
   */
  if (clientsock_nb("10.0.0.1", 21, &sock) < 0) {
    HPHP::Logger::Error("Can't open socket to determine local address");
    return -1;
  }

  if (getsockname(sock, (struct sockaddr *)&addr, &len)) {
    HPHP::Logger::Error("Can't determine local address of connection");
    return -1;
  }

  close(sock);
  memcpy(addrbuf, &addr.sin_addr, sizeof(struct in_addr));
  return 0;
}

