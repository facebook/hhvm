/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <test/test_ext_socket.h>
#include <runtime/ext/ext_socket.h>
#include <runtime/ext/ext_network.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtSocket::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_socket_create);
  RUN_TEST(test_socket_create_listen);
  RUN_TEST(test_socket_create_pair);
  RUN_TEST(test_socket_get_option);
  RUN_TEST(test_socket_getpeername);
  RUN_TEST(test_socket_getsockname);
  RUN_TEST(test_socket_set_block);
  RUN_TEST(test_socket_set_nonblock);
  RUN_TEST(test_socket_set_option);
  RUN_TEST(test_socket_connect);
  RUN_TEST(test_socket_bind);
  RUN_TEST(test_socket_listen);
  RUN_TEST(test_socket_select);
  RUN_TEST(test_socket_server);
  RUN_TEST(test_socket_accept);
  RUN_TEST(test_socket_read);
  RUN_TEST(test_socket_write);
  RUN_TEST(test_socket_send);
  RUN_TEST(test_socket_sendto);
  RUN_TEST(test_socket_recv);
  RUN_TEST(test_socket_recvfrom);
  RUN_TEST(test_socket_shutdown);
  RUN_TEST(test_socket_close);
  RUN_TEST(test_socket_strerror);
  RUN_TEST(test_socket_last_error);
  RUN_TEST(test_socket_clear_error);

  return ret;
}

// so we run on different range of ports every time
static int get_random_port() {
  static int base = -1;
  if (base == -1) {
    base = 12345 + (int)((time(0) * 100) % 30000);
  }
  return ++base;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtSocket::test_socket_create() {
  VERIFY(!same(f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP), false));
  return Count(true);
}

bool TestExtSocket::test_socket_create_listen() {
  int port = get_random_port();
  f_socket_create_listen(port);
  return Count(true);
}

bool TestExtSocket::test_socket_create_pair() {
  Variant fds;
  VERIFY(f_socket_create_pair(k_AF_UNIX, k_SOCK_STREAM, 0, ref(fds)));
  VS(fds.toArray().size(), 2);
  VERIFY(more(fds[0], 0));
  VERIFY(more(fds[1], 0));
  return Count(true);
}

bool TestExtSocket::test_socket_get_option() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VS(f_socket_get_option(s, k_SOL_SOCKET, k_SO_TYPE), k_SOCK_STREAM);
  return Count(true);
}

bool TestExtSocket::test_socket_getpeername() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_connect(s, "facebook.com", 80));

  Variant address;
  VERIFY(f_socket_getpeername(s, ref(address)));

  VERIFY(!address.toString().empty());
  return Count(true);
}

bool TestExtSocket::test_socket_getsockname() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_connect(s, "facebook.com", 80));

  Variant address;
  VERIFY(f_socket_getsockname(s, ref(address)));

  VERIFY(!address.toString().empty());
  return Count(true);
}

bool TestExtSocket::test_socket_set_block() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_set_block(s));
  return Count(true);
}

bool TestExtSocket::test_socket_set_nonblock() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_set_nonblock(s));
  return Count(true);
}

bool TestExtSocket::test_socket_set_option() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_set_option(s, k_SOL_SOCKET, k_SO_RCVTIMEO,
                             CREATE_MAP2("sec", 1, "usec", 0)));
  return Count(true);
}

bool TestExtSocket::test_socket_connect() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_connect(s, "facebook.com", 80));
  return Count(true);
}

bool TestExtSocket::test_socket_bind() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  int port = get_random_port();
  VERIFY(f_socket_bind(s, "127.0.0.1", port));
  VERIFY(f_socket_listen(s));
  return Count(true);
}

bool TestExtSocket::test_socket_listen() {
  Variant server = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  int port = get_random_port();
  VERIFY(f_socket_bind(server, "127.0.0.1", port));
  VERIFY(f_socket_listen(server));

  Variant client = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_connect(client, "127.0.0.1", port));

  Variant s = f_socket_accept(server);
  VERIFY(f_socket_write(client, "testing"));

  // this could fail with shorter returns, but it never does...
  VS(f_socket_read(s, 100), "testing");
  return Count(true);
}

bool TestExtSocket::test_socket_select() {
  Variant server = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  int port = get_random_port();
  VERIFY(f_socket_bind(server, "127.0.0.1", port));
  VERIFY(f_socket_listen(server));

  Variant client = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_connect(client, "127.0.0.1", port));

  Variant s = f_socket_accept(server);

  Variant reads = CREATE_VECTOR1(s);
  VS(f_socket_select(ref(reads), null, null, 1, 0), 0);

  VERIFY(f_socket_write(client, "testing"));
  reads = CREATE_VECTOR1(s);
  VS(f_socket_select(ref(reads), null, null, 1, 0), 1);
  return Count(true);
}

bool TestExtSocket::test_socket_server() {
  int port = get_random_port();
  Variant server = f_socket_server("127.0.0.1", port);
  VERIFY(!same(server, false));

  Variant client = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_connect(client, "127.0.0.1", port));

  Variant s = f_socket_accept(server);

  Variant reads = CREATE_VECTOR1(s);
  VS(f_socket_select(ref(reads), null, null, 1, 0), 0);

  VERIFY(f_socket_write(client, "testing"));
  reads = CREATE_VECTOR1(s);
  VS(f_socket_select(ref(reads), null, null, 1, 0), 1);
  return Count(true);
}

bool TestExtSocket::test_socket_accept() {
  // tested with test_socket_listen
  return Count(true);
}

bool TestExtSocket::test_socket_read() {
  // tested with test_socket_listen
  return Count(true);
}

bool TestExtSocket::test_socket_write() {
  // tested with test_socket_listen
  return Count(true);
}

bool TestExtSocket::test_socket_send() {
  Variant server = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  int port = get_random_port();
  VERIFY(f_socket_bind(server, "127.0.0.1", port));
  VERIFY(f_socket_listen(server));

  Variant client = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_connect(client, "127.0.0.1", port));

  Variant s = f_socket_accept(server);
  String text = "testing";
  VERIFY(f_socket_send(client, text, 4, 0));

  Variant buffer;
  VERIFY(f_socket_recv(s, ref(buffer), 100, 0));
  VS(buffer, "test");
  return Count(true);
}

bool TestExtSocket::test_socket_sendto() {
  Variant server = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  int port = get_random_port();
  VERIFY(f_socket_bind(server, "127.0.0.1", port));
  VERIFY(f_socket_listen(server));

  Variant client = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  VERIFY(f_socket_connect(client, "127.0.0.1", port));

  Variant s = f_socket_accept(server);
  String text = "testing";
  VERIFY(f_socket_sendto(client, text, 4, 0, "127.0.0.1", port));

  Variant buffer;
  Variant name, vport;
  VERIFY(f_socket_recvfrom(s, ref(buffer), 100, 0, ref(name), ref(vport)));
  VS(buffer, "test");
  return Count(true);
}

bool TestExtSocket::test_socket_recv() {
  // tested with test_socket_send
  return Count(true);
}

bool TestExtSocket::test_socket_recvfrom() {
  // tested with test_socket_sendto
  return Count(true);
}

bool TestExtSocket::test_socket_shutdown() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  int port = get_random_port();
  VERIFY(f_socket_bind(s, "127.0.0.1", port));
  VERIFY(f_socket_listen(s));
  VERIFY(f_socket_shutdown(s));
  return Count(true);
}

bool TestExtSocket::test_socket_close() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  int port = get_random_port();
  VERIFY(f_socket_bind(s, "127.0.0.1", port));
  VERIFY(f_socket_listen(s));
  f_socket_close(s);
  return Count(true);
}

bool TestExtSocket::test_socket_strerror() {
  Variant s = f_socket_create(k_AF_INET, k_SOCK_STREAM, k_SOL_TCP);
  f_socket_bind(s, "127.0.0.1", 80);
  if (same(f_socket_last_error(s), 13)) {
    VS(f_socket_strerror(13), "Permission denied");
    f_socket_clear_error(s);
  }
  VS(f_socket_last_error(s), 0);
  return Count(true);
}

bool TestExtSocket::test_socket_last_error() {
  // tested with test_socket_strerror
  return Count(true);
}

bool TestExtSocket::test_socket_clear_error() {
  // tested with test_socket_strerror
  return Count(true);
}
