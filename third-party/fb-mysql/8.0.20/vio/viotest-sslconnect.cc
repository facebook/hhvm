/* Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "all.h"

#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void fatal_error(const char *r) {
  perror(r);
  exit(0);
}

void print_usage() {
  printf("viotest-sslconnect: testing SSL virtual IO. Usage:\n");
  printf("viotest-sslconnect key cert\n");
}

int main(int argc, char **argv) {
  char *key = 0;
  char *cert = 0;

  if (argc < 3) {
    print_usage();
    return 1;
  }

  char ip[4] = {127, 0, 0, 1};
  unsigned long addr = (unsigned long)((unsigned long)ip[0] << 24L) |
                       ((unsigned long)ip[1] << 16L) |
                       ((unsigned long)ip[2] << 8L) | ((unsigned long)ip[3]);
  int fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0) fatal_error("socket");
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(4433);
  sa.sin_addr.s_addr = htonl(addr);
  int sa_size = sizeof sa;
  if (connect(fd, reinterpret_cast<const sockaddr *>(&sa), sa_size) == -1)
    fatal_error("connect");
  key = argv[1];
  cert = argv[2];
  printf("Key  : %s\n", key);
  printf("Cert : %s\n", cert);

  VIO_NS::VioSSLConnectorFd *ssl_connector =
      new VIO_NS::VioSSLConnectorFd(cert, key, 0, 0);

  VIO_NS::VioSSL *vio = ssl_connector->connect(fd);

  char xbuf[100];
  int r = vio->read(xbuf, sizeof(xbuf));
  if (r <= 0) {
    delete ssl_connector;
    delete vio;
    fatal_error("client:SSL_read");
  }
  xbuf[r] = 0;
  printf("client:got %s\n", xbuf);
  delete vio;
  delete ssl_connector;
  return 0;
}
