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

#if 0
static void
fatal_error(	const char*	r)
{
	perror(r);
	exit(0);
}
#endif

typedef struct {
  int sd;
  struct st_VioSSLFd *ssl_acceptor;
} TH_ARGS;

static void do_ssl_stuff(TH_ARGS *args) {
  const char *s = "Huhuhuhuuu";
  Vio *server_vio;
  int err;
  unsigned long ssl_error;
  DBUG_TRACE;

  server_vio = vio_new(args->sd, VIO_TYPE_TCPIP, TRUE);

  /* ----------------------------------------------- */
  /* TCP connection is ready. Do server side SSL. */

  err = write(server_vio->sd, (uchar *)s, strlen(s));
  sslaccept(args->ssl_acceptor, server_vio, 60L, &ssl_error);
  err = server_vio->write(server_vio, (uchar *)s, strlen(s));
}

static void *client_thread(void *arg) {
  my_thread_init();
  do_ssl_stuff((TH_ARGS *)arg);
  return 0;
}

int main(int argc MY_ATTRIBUTE((unused)), char **argv) {
  char server_key[] = "../SSL/server-key.pem",
       server_cert[] = "../SSL/server-cert.pem";
  char ca_file[] = "../SSL/cacert.pem", *ca_path = 0, *cipher = 0;
  struct st_VioSSLFd *ssl_acceptor;
  pthread_t th;
  TH_ARGS th_args;

  struct sockaddr_in sa_serv;
  struct sockaddr_in sa_cli;
  int listen_sd;
  int err;
  socket_len_t client_len;
  int reuseaddr = 1; /* better testing, uh? */

  MY_INIT(argv[0]);
  DBUG_PROCESS(argv[0]);
  DBUG_PUSH(default_dbug_option);

  printf("Server key/cert : %s/%s\n", server_key, server_cert);
  if (ca_file != 0) printf("CAfile          : %s\n", ca_file);
  if (ca_path != 0) printf("CApath          : %s\n", ca_path);

  th_args.ssl_acceptor = ssl_acceptor =
      new_VioSSLAcceptorFd(server_key, server_cert, ca_file, ca_path, cipher);

  /* ----------------------------------------------- */
  /* Prepare TCP socket for receiving connections */

  listen_sd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
             sizeof(&reuseaddr));

  memset(&sa_serv, '\0', sizeof(sa_serv));
  sa_serv.sin_family = AF_INET;
  sa_serv.sin_addr.s_addr = INADDR_ANY;
  sa_serv.sin_port = htons(1111); /* Server Port number */

  err = bind(listen_sd, (struct sockaddr *)&sa_serv, sizeof(sa_serv));

  /* Receive a TCP connection. */

  err = listen(listen_sd, 5);
  client_len = sizeof(sa_cli);
  th_args.sd = accept(listen_sd, (struct sockaddr *)&sa_cli, &client_len);
  close(listen_sd);

  printf("Connection from %lx, port %x\n", (long)sa_cli.sin_addr.s_addr,
         sa_cli.sin_port);

  /* ----------------------------------------------- */
  /* TCP connection is ready. Do server side SSL. */

  err = pthread_create(&th, NULL, client_thread, (void *)&th_args);
  DBUG_PRINT("info", ("pthread_create: %d", err));
  pthread_join(th, NULL);

#if 0
	if (err<=0) {
		my_free(ssl_acceptor);
		fatal_error("server:SSL_write");
	}
#endif /* 0 */

  my_free(ssl_acceptor);
  return 0;
}
