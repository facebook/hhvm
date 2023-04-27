/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <m_ctype.h>
#include <m_string.h>
#include <my_dir.h>
#include <my_getopt.h>
#include <my_sys.h>
#include <signal.h>
#include <violite.h>

#include "errmsg.h"
#include "mysql.h"

const char *VER = "0.2";

#ifndef DBUG_OFF
const char *default_dbug_option = "d:t:O,-";
#endif

void fatal_error(const char *r) {
  perror(r);
  exit(0);
}

int main(int argc MY_ATTRIBUTE((unused)), char **argv) {
  char client_key[] = "../SSL/client-key.pem",
       client_cert[] = "../SSL/client-cert.pem";
  char ca_file[] = "../SSL/cacert.pem", *ca_path = 0, *cipher = 0;
  struct st_VioSSLFd *ssl_connector = 0;
  struct sockaddr_in sa;
  Vio *client_vio = 0;
  int err;
  char xbuf[100] = "Ohohhhhoh1234";
  enum enum_ssl_init_error ssl_init_error;
  unsigned long ssl_error;

  MY_INIT(argv[0]);
  DBUG_PROCESS(argv[0]);
  DBUG_PUSH(default_dbug_option);

  printf("Client key/cert : %s/%s\n", client_key, client_cert);
  if (ca_file != 0) printf("CAfile          : %s\n", ca_file);
  if (ca_path != 0) printf("CApath          : %s\n", ca_path);

  ssl_connector = new_VioSSLConnectorFd(client_key, client_cert, ca_file,
                                        ca_path, cipher, &ssl_init_error);
  if (!ssl_connector) {
    fatal_error("client:new_VioSSLConnectorFd failed");
  }

  /* ----------------------------------------------- */
  /* Create a socket and connect to server using normal socket calls. */

  client_vio = vio_new(socket(AF_INET, SOCK_STREAM, 0), VIO_TYPE_TCPIP, TRUE);

  memset(&sa, '\0', sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Server IP */
  sa.sin_port = htons(1111);                   /* Server Port number */

  err = connect(client_vio->sd, (struct sockaddr *)&sa, sizeof(sa));

  /* ----------------------------------------------- */
  /* Now we have TCP conncetion. Start SSL negotiation. */
  read(client_vio->sd, xbuf, sizeof(xbuf));
  sslconnect(ssl_connector, client_vio, 60L, &ssl_error);
  err = vio_read(client_vio, xbuf, sizeof(xbuf));
  if (err <= 0) {
    my_free(ssl_connector);
    fatal_error("client:SSL_read");
  }
  xbuf[err] = 0;
  printf("client:got %s\n", xbuf);
  my_free(client_vio);
  my_free(ssl_connector);
  return 0;
}
