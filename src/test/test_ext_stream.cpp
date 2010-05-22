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

#include <test/test_ext_stream.h>
#include <runtime/ext/ext_stream.h>
#include <runtime/ext/ext_socket.h>
#include <runtime/ext/ext_file.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

bool TestExtStream::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_stream_context_create);
  RUN_TEST(test_stream_context_get_default);
  RUN_TEST(test_stream_context_get_options);
  RUN_TEST(test_stream_context_set_option);
  RUN_TEST(test_stream_context_set_param);
  RUN_TEST(test_stream_copy_to_stream);
  RUN_TEST(test_stream_encoding);
  RUN_TEST(test_stream_bucket_append);
  RUN_TEST(test_stream_bucket_prepend);
  RUN_TEST(test_stream_bucket_make_writeable);
  RUN_TEST(test_stream_bucket_new);
  RUN_TEST(test_stream_filter_register);
  RUN_TEST(test_stream_filter_remove);
  RUN_TEST(test_stream_filter_append);
  RUN_TEST(test_stream_filter_prepend);
  RUN_TEST(test_stream_get_contents);
  RUN_TEST(test_stream_get_filters);
  RUN_TEST(test_stream_get_line);
  RUN_TEST(test_stream_get_meta_data);
  RUN_TEST(test_stream_get_transports);
  RUN_TEST(test_stream_get_wrappers);
  RUN_TEST(test_stream_register_wrapper);
  RUN_TEST(test_stream_wrapper_register);
  RUN_TEST(test_stream_wrapper_restore);
  RUN_TEST(test_stream_wrapper_unregister);
  RUN_TEST(test_stream_resolve_include_path);
  RUN_TEST(test_stream_select);
  RUN_TEST(test_stream_set_blocking);
  RUN_TEST(test_stream_set_timeout);
  RUN_TEST(test_stream_set_write_buffer);
  RUN_TEST(test_set_file_buffer);
  RUN_TEST(test_stream_socket_accept);
  RUN_TEST(test_stream_socket_server);
  RUN_TEST(test_stream_socket_client);
  RUN_TEST(test_stream_socket_enable_crypto);
  RUN_TEST(test_stream_socket_get_name);
  RUN_TEST(test_stream_socket_pair);
  RUN_TEST(test_stream_socket_recvfrom);
  RUN_TEST(test_stream_socket_sendto);
  RUN_TEST(test_stream_socket_shutdown);

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

bool TestExtStream::test_stream_context_create() {
  f_stream_context_create();
  return Count(true);
}

bool TestExtStream::test_stream_context_get_default() {
  try {
    f_stream_context_get_default();
  } catch (NotImplementedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_context_get_options() {
  try {
    f_stream_context_get_options(Object());
  } catch (NotImplementedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_context_set_option() {
  try {
    f_stream_context_set_option(Object(), "");
  } catch (NotImplementedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_context_set_param() {
  try {
    f_stream_context_set_param(Object(), Array());
  } catch (NotImplementedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_copy_to_stream() {
  Variant src = f_fopen("test/test_ext_file.txt", "r");
  Variant dest = f_fopen("test/test_ext_file.tmp", "w");
  f_stream_copy_to_stream(src, dest);
  f_fclose(dest);

  Variant f = f_fopen("test/test_ext_file.tmp", "r");
  VS(f_fgets(f), "Testing Ext File\n");
  return Count(true);
}

bool TestExtStream::test_stream_encoding() {
  try {
    f_stream_encoding(Object());
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_bucket_append() {
  try {
    f_stream_bucket_append(Object(), Object());
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_bucket_prepend() {
  try {
    f_stream_bucket_prepend(Object(), Object());
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_bucket_make_writeable() {
  try {
    f_stream_bucket_make_writeable(Object());
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_bucket_new() {
  try {
    f_stream_bucket_new(Object(), "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_filter_register() {
  try {
    f_stream_filter_register("", "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_filter_remove() {
  try {
    f_stream_filter_remove(Object());
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_filter_append() {
  try {
    f_stream_filter_append(Object(), "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_filter_prepend() {
  try {
    f_stream_filter_prepend(Object(), "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_get_contents() {
  Variant f = f_fopen("test/test_ext_file.txt", "r");
  VS(f_stream_get_contents(f), "Testing Ext File\n");
  return Count(true);
}

bool TestExtStream::test_stream_get_filters() {
  try {
    f_stream_get_filters();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_get_line() {
  Variant f = f_fopen("test/test_ext_file.txt", "r");
  VS(f_stream_get_line(f), "Testing Ext File\n");
  return Count(true);
}

bool TestExtStream::test_stream_get_meta_data() {
  int port = get_random_port();
  string address = string("127.0.0.1:") + boost::lexical_cast<string>(port);

  Variant server = f_stream_socket_server(address);
  Variant client = f_stream_socket_client(address);

  f_stream_set_timeout(client, 0, 500 * 1000); // 500ms
  Variant line = f_fgets(client);
  Variant meta = f_stream_get_meta_data(client);
  VS(meta["timed_out"], true);

  return Count(true);
}

bool TestExtStream::test_stream_get_transports() {
  VERIFY(f_stream_get_transports().size() > 0);
  return Count(true);
}

bool TestExtStream::test_stream_get_wrappers() {
  try {
    f_stream_get_wrappers();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_register_wrapper() {
  try {
    f_stream_register_wrapper("", "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_wrapper_register() {
  try {
    f_stream_wrapper_register("", "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_wrapper_restore() {
  try {
    f_stream_wrapper_restore("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_wrapper_unregister() {
  try {
    f_stream_wrapper_unregister("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_resolve_include_path() {
  try {
    f_stream_resolve_include_path("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_select() {
  Variant f = f_fopen("test/test_ext_file.txt", "r");
  Variant reads = CREATE_VECTOR1(f);
  VERIFY(!same(f_stream_select(ref(reads), null, null, 0, 0), false));
  return Count(true);
}

bool TestExtStream::test_stream_set_blocking() {
  Variant f = f_fopen("test/test_ext_file.txt", "r");
  VERIFY(f_stream_set_blocking(f, 0));
  return Count(true);
}

bool TestExtStream::test_stream_set_timeout() {
  Variant f = f_fopen("test/test_ext_file.txt", "r");
  f_stream_set_timeout(f, 0);
  return Count(true);
}

bool TestExtStream::test_stream_set_write_buffer() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_stream_set_write_buffer(f, 1 /* PHP_STREAM_BUFFER_LINE */);
  f_fputs(f, "testing\nanother line\n");
  return Count(true);
}

bool TestExtStream::test_set_file_buffer() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_set_file_buffer(f, 1 /* PHP_STREAM_BUFFER_LINE */);
  f_fputs(f, "testing\nanother line\n");
  return Count(true);
}

bool TestExtStream::test_stream_socket_accept() {
  // tested in test_stream_socket_recvfrom
  return Count(true);
}

bool TestExtStream::test_stream_socket_server() {
  // tested in test_stream_socket_recvfrom
  return Count(true);
}

bool TestExtStream::test_stream_socket_client() {
  // tested in test_stream_socket_recvfrom
  return Count(true);
}

bool TestExtStream::test_stream_socket_enable_crypto() {
  try {
    f_stream_socket_enable_crypto(Object(), true);
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtStream::test_stream_socket_get_name() {
  Variant client = f_stream_socket_client("facebook.com:80");
  VERIFY(!f_stream_socket_get_name(client, true).toString().empty());
  VERIFY(!f_stream_socket_get_name(client, false).toString().empty());
  return Count(true);
}

bool TestExtStream::test_stream_socket_pair() {
  Variant fds = f_stream_socket_pair(k_AF_UNIX, k_SOCK_STREAM, 0);
  VS(fds.toArray().size(), 2);
  VERIFY(more(fds[0], 0));
  VERIFY(more(fds[1], 0));
  return Count(true);
}

bool TestExtStream::test_stream_socket_recvfrom() {
  vector<string> addresses;
  int port = get_random_port();
  addresses.push_back("127.0.0.1:" + boost::lexical_cast<string>(port)); // TCP
  addresses.push_back("unix:///tmp/test_stream.sock"); // UNIX domain
  unlink("/tmp/test_stream.sock");

  for (unsigned int i = 0; i < addresses.size(); i++) {
    const string &address = addresses[i];

    Variant server = f_stream_socket_server(address);
    Variant client = f_stream_socket_client(address);

    Variant s = f_stream_socket_accept(server);
    String text = "testing";
    if (i == 1) {
      VERIFY(f_socket_send(client, text, 7, 0));
    } else {
      VERIFY(f_stream_socket_sendto(client, text, 0, address));
    }

    Variant buffer = f_stream_socket_recvfrom(s, 100);
    VS(buffer, "testing");
  }
  return Count(true);
}

bool TestExtStream::test_stream_socket_sendto() {
  // tested in test_stream_socket_recvfrom
  return Count(true);
}

bool TestExtStream::test_stream_socket_shutdown() {
  int port = get_random_port();
  string address = "127.0.0.1:" + boost::lexical_cast<string>(port);
  Variant server = f_stream_socket_server(address);
  VERIFY(f_stream_socket_shutdown(server, 0));
  return Count(true);
}
