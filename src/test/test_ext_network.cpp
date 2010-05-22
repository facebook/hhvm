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

#include <test/test_ext_network.h>
#include <runtime/ext/ext_network.h>
#include <runtime/ext/ext_file.h>
#include <runtime/ext/ext_string.h>
#include <runtime/base/util/string_buffer.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtNetwork::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_gethostbyaddr);
  RUN_TEST(test_gethostbyname);
  RUN_TEST(test_gethostbynamel);
  RUN_TEST(test_getprotobyname);
  RUN_TEST(test_getprotobynumber);
  RUN_TEST(test_getservbyname);
  RUN_TEST(test_getservbyport);
  RUN_TEST(test_inet_ntop);
  RUN_TEST(test_inet_pton);
  RUN_TEST(test_ip2long);
  RUN_TEST(test_long2ip);
  RUN_TEST(test_dns_check_record);
  RUN_TEST(test_checkdnsrr);
  RUN_TEST(test_dns_get_record);
  RUN_TEST(test_dns_get_mx);
  RUN_TEST(test_getmxrr);
  RUN_TEST(test_fsockopen);
  RUN_TEST(test_pfsockopen);
  RUN_TEST(test_socket_get_status);
  RUN_TEST(test_socket_set_blocking);
  RUN_TEST(test_socket_set_timeout);
  RUN_TEST(test_header);
  RUN_TEST(test_headers_list);
  RUN_TEST(test_headers_sent);
  RUN_TEST(test_header_remove);
  RUN_TEST(test_setcookie);
  RUN_TEST(test_setrawcookie);
  RUN_TEST(test_define_syslog_variables);
  RUN_TEST(test_openlog);
  RUN_TEST(test_closelog);
  RUN_TEST(test_syslog);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtNetwork::test_gethostbyaddr() {
  VS(f_gethostbyaddr("127.0.0.1"), "localhost.localdomain");
  return Count(true);
}

bool TestExtNetwork::test_gethostbyname() {
  VS(f_gethostbyname("localhost"), "127.0.0.1");
  return Count(true);
}

bool TestExtNetwork::test_gethostbynamel() {
  VS(f_gethostbynamel("localhost"), CREATE_VECTOR1("127.0.0.1"));
  return Count(true);
}

bool TestExtNetwork::test_getprotobyname() {
  VS(f_getprotobyname("tcp"), 6);
  return Count(true);
}

bool TestExtNetwork::test_getprotobynumber() {
  VS(f_getprotobynumber(6), "tcp");
  return Count(true);
}

bool TestExtNetwork::test_getservbyname() {
  VS(f_getservbyname("http", "tcp"), 80);
  return Count(true);
}

bool TestExtNetwork::test_getservbyport() {
  VS(f_getservbyport(80, "tcp"), "http");
  return Count(true);
}

bool TestExtNetwork::test_inet_ntop() {
  String packed = f_chr(127) + f_chr(0) + f_chr(0) + f_chr(1);
  VS(f_inet_ntop(packed), "127.0.0.1");

  packed = f_str_repeat(f_chr(0), 15) + f_chr(1);
  VS(f_inet_ntop(packed), "::1");
  return Count(true);
}

bool TestExtNetwork::test_inet_pton() {
  String packed = f_chr(127) + f_chr(0) + f_chr(0) + f_chr(1);
  VS(f_inet_pton("127.0.0.1"), packed);

  packed = f_str_repeat(f_chr(0), 15) + f_chr(1);
  VS(f_inet_pton("::1"), packed);
  return Count(true);
}

bool TestExtNetwork::test_ip2long() {
  VS(f_ip2long("127.0.0.1"), 2130706433);
  return Count(true);
}

bool TestExtNetwork::test_long2ip() {
  VS(f_long2ip(2130706433), "127.0.0.1");
  return Count(true);
}

bool TestExtNetwork::test_dns_check_record() {
  VERIFY(f_dns_check_record("facebook.com"));
  return Count(true);
}

bool TestExtNetwork::test_checkdnsrr() {
  VERIFY(f_checkdnsrr("facebook.com"));
  return Count(true);
}

bool TestExtNetwork::test_dns_get_record() {
  VERIFY(!f_dns_get_record("facebook.com", k_DNS_A).toArray().empty());
  return Count(true);
}

bool TestExtNetwork::test_dns_get_mx() {
  Variant hosts;
  VERIFY(f_dns_get_mx("facebook.com", ref(hosts)));
  VERIFY(!hosts.toArray().empty());
  return Count(true);
}

bool TestExtNetwork::test_getmxrr() {
  Variant hosts;
  VERIFY(f_getmxrr("facebook.com", ref(hosts)));
  VERIFY(!hosts.toArray().empty());
  return Count(true);
}

bool TestExtNetwork::test_fsockopen() {
  {
    Variant f = f_fsockopen("facebook.com", 80);
    VERIFY(!same(f, false));
    f_fputs(f, "GET / HTTP/1.0\n\n");
    VERIFY(!f_fread(f, 15).toString().empty());
  }
  {
    Variant f = f_fsockopen("ssl://www.facebook.com", 443);
    VERIFY(!same(f, false));
    f_fwrite(f,
             "GET / HTTP/1.1\r\n"
             "Host: www.facebook.com\r\n"
             "Connection: Close\r\n"
             "\r\n");
    StringBuffer response;
    while (!same(f_feof(f), true)) {
      Variant line = f_fgets(f, 128);
      response.append(line.toString());
    }
    VERIFY(!response.detach().empty());
  }
  return Count(true);
}

bool TestExtNetwork::test_pfsockopen() {
  Variant f = f_pfsockopen("facebook.com", 80);
  VERIFY(!same(f, false));
  f_fputs(f, "GET / HTTP/1.0\n\n");
  VERIFY(!f_fread(f, 15).toString().empty());
  return Count(true);
}

bool TestExtNetwork::test_socket_get_status() {
  VS(f_socket_get_status(Object()), null);
  return Count(true);
}

bool TestExtNetwork::test_socket_set_blocking() {
  Variant f = f_fsockopen("facebook.com", 80);
  f_socket_set_blocking(f, 0);
  return Count(true);
}

bool TestExtNetwork::test_socket_set_timeout() {
  Variant f = f_fsockopen("facebook.com", 80);
  f_socket_set_timeout(f, 0);
  return Count(true);
}

bool TestExtNetwork::test_header() {
  f_header("Location: http://www.facebook.com");
  return Count(true);
}

bool TestExtNetwork::test_headers_list() {
  f_header("Location: http://www.facebook.com");
  VS(f_headers_list(), Array());
  return Count(true);
}

bool TestExtNetwork::test_headers_sent() {
  f_header("Location: http://www.facebook.com");
  VERIFY(!f_headers_sent());
  return Count(true);
}

bool TestExtNetwork::test_header_remove() {
  f_header_remove("name");
  f_header_remove();
  return Count(true);
}

bool TestExtNetwork::test_setcookie() {
  VERIFY(!f_setcookie("name", "value"));
  return Count(true);
}

bool TestExtNetwork::test_setrawcookie() {
  VERIFY(!f_setrawcookie("name", "value"));
  return Count(true);
}

bool TestExtNetwork::test_define_syslog_variables() {
  f_define_syslog_variables();
  return Count(true);
}

bool TestExtNetwork::test_openlog() {
  f_openlog("TestExtNetwork", k_LOG_ODELAY, k_LOG_USER);
  f_syslog(k_LOG_INFO, "testing");
  f_closelog();
  return Count(true);
}

bool TestExtNetwork::test_closelog() {
  f_openlog("TestExtNetwork", k_LOG_ODELAY, k_LOG_USER);
  f_syslog(k_LOG_INFO, "testing");
  f_closelog();
  return Count(true);
}

bool TestExtNetwork::test_syslog() {
  f_openlog("TestExtNetwork", k_LOG_ODELAY, k_LOG_USER);
  f_syslog(k_LOG_INFO, "testing");
  f_closelog();
  return Count(true);
}
