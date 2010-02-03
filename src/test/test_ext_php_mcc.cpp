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

#include <test/test_ext_php_mcc.h>
#include <cpp/ext/ext_array.h>
#include <util/async_func.h>
#include <util/process.h>

using namespace std;
using namespace boost;

#define MEMCACHED "memcached-1.2.3e"
///////////////////////////////////////////////////////////////////////////////
// helpers

bool TestExtPhp_mcc::IsPortInUse(int port) {
  char sport[256];
  snprintf(sport, sizeof(sport), ":%d", port);
  string out, err;
  const char *argv[] = {"", "-t", "-i", sport, NULL};
  Process::Exec("lsof", argv, NULL, out, &err);
  return !out.empty();
}

int TestExtPhp_mcc::FindFreePort() {
  for (int i = 0; i < 1000; i++) {
    int port = (abs(rand()) % (65536 - 1025)) + 1025;
    if (!IsPortInUse(port)) {
      return port;
    }
  }
  return 0;
}

void TestExtPhp_mcc::run_server() {
  m_server_tcp_port = FindFreePort();
  m_server_udp_port = FindFreePort();
  if (m_server_tcp_port == 0 || m_server_udp_port == 0) {
    printf("ERROR: unable to find free ports for memcached server\n");
    return;
  }
  string tcp = boost::lexical_cast<string>(m_server_tcp_port);
  string udp = boost::lexical_cast<string>(m_server_udp_port);

  string out, err;
  const char *argv[] = {"", "-p", tcp.c_str(), "-U", udp.c_str(),
                        "-m", "64", NULL};
  string cmd = string("../external/memcached/") + MEMCACHED;
  Process::Exec(cmd.c_str(), argv, NULL, out, &err);
  if (!err.empty()) {
    printf("ERROR: unable to start memcached server: %s\n", err.c_str());
  }
}

bool TestExtPhp_mcc::get_client(bool consistent_hashing, p_phpmcc &mcc) {
  mcc = p_phpmcc(new c_phpmcc());
  mcc->create("test", 0);

  const char *sp = "wildcard";
  const char *s = "wildcard-0";

  VERIFY(mcc->t_add_serverpool(sp, consistent_hashing));
  VS(mcc->t_serverpool_get_consistent_hashing_enabled(sp), consistent_hashing);
  mcc->t___set("default_serverpool", sp);
  VERIFY(mcc->t_add_server(s));
  VERIFY(mcc->t_serverpool_add_server(sp, s));
  VERIFY(mcc->t_add_accesspoint(s, "127.0.0.1", m_server_tcp_port));
  VERIFY(mcc->t_add_accesspoint(s, "127.0.0.1", m_server_udp_port,
                                k_MCC_IPPROTO_UDP));

  Variant aps = mcc->t_get_accesspoints(s);
  VS(f_in_array(String(string("tcp:127.0.0.1:") +
                       lexical_cast<string>(m_server_tcp_port)), aps), true);
  VS(f_in_array(String(string("udp:127.0.0.1:") +
                       lexical_cast<string>(m_server_udp_port)), aps), true);

  VS(mcc->t_serverpool_get_servers(sp), CREATE_MAP1(s, aps));
  VS(mcc->t___get("serverpools"), CREATE_MAP1(sp, CREATE_MAP1(s, aps)));
  return true;
}

///////////////////////////////////////////////////////////////////////////////

TestExtPhp_mcc::TestExtPhp_mcc() : m_server_tcp_port(0), m_server_udp_port(0) {
}

bool TestExtPhp_mcc::RunTests(const std::string &which) {
  bool ret = true;
  return ret; // disabling it for now, as it made several others fail

  AsyncFunc<TestExtPhp_mcc> func(this, &TestExtPhp_mcc::run_server);
  func.start();
  while (m_server_tcp_port == 0 || m_server_udp_port == 0 ||
         !IsPortInUse(m_server_tcp_port) ||
         !IsPortInUse(m_server_udp_port)) {
    usleep(20000);
  }
  printf("memcached started at (tcp: %d, udp: %d)\n",
         m_server_tcp_port, m_server_udp_port);

  RUN_TEST(test_basic);

  Util::ssystem((string("killall ") + MEMCACHED).c_str());
  while (IsPortInUse(m_server_tcp_port) && IsPortInUse(m_server_udp_port)) {
    usleep(20000);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtPhp_mcc::test_basic() {
  p_phpmcc mcc;
  if (!get_client(false, mcc)) return false;

  const char *k = "key";
  const char *v = "value";
  Array kv = CREATE_MAP1(k, v);
  Array kvs = CREATE_MAP3("k0", "v0", "k1", "v1", "k2", "v2");

  VS(mcc->t_get(k), null);
  VS(mcc->t_delete(k), false);
  mcc->t_set(k, v);
  VS(mcc->t_get(k), v);
  VS(mcc->t_delete(k), true);
  VS(mcc->t_get(k), null);
  VS(mcc->t_delete(k), false);

  mcc->t_set(k, v);
  VS(mcc->t_get(kv), kv);

  v = "new value";
  VERIFY(!mcc->t_add(k, v));
  VERIFY(mcc->t_replace(k, v));
  kv = CREATE_MAP1(k, v);
  VS(mcc->t_get(kv), kv);

  Variant result = mcc->t_delete(CREATE_VECTOR1(k));
  VS(result[k], true);
  VS(mcc->t_get(Array::Create()), Array::Create());
  result = mcc->t_delete(CREATE_VECTOR1(k));
  VS(result[k], false);

  mcc->t_set("k0", "v0");
  mcc->t_set("k1", "v1");
  mcc->t_set("k2", "v2");
  VS(mcc->t_get(kvs), kvs);
  VS(mcc->t_get(f_array_keys(kvs)), kvs);
  VS(mcc->t_get_multi(f_array_keys(kvs)), kvs);

  VS(mcc->t_delete(f_array_keys(kvs)),
     CREATE_MAP3("k0", true, "k1", true, "k2", true));
  VS(mcc->t_delete(f_array_keys(kvs)),
     CREATE_MAP3("k0", false, "k1", false, "k2", false));
  VS(mcc->t_get(kvs), Array::Create());

  v = "0";
  kv = CREATE_MAP1(k, v);
  mcc->t_set(k, v);
  VS(mcc->t_get(kv), kv);
  VS(mcc->t_incr(k, 1), 1);
  VS(mcc->t_incr(k, 1000), 1001);
  VS(mcc->t_decr(k, 1), 1000);
  VS(mcc->t_decr(k, 1001), 0);

  v = "notanumber";
  kv = CREATE_MAP1(k, v);
  mcc->t_set(k, v);
  VS(mcc->t_get(kv), kv);
  VS(mcc->t_incr(k, 1), 1);

  v = "notanumber";
  kv = CREATE_MAP1(k, v);
  mcc->t_set(k, v);
  VS(mcc->t_get(kv), kv);
  VS(mcc->t_decr(k, 1), 0);

  mcc->t_delete(k);
  VS(mcc->t_incr("test.non-existent-key", 1), false);
  VS(mcc->t_decr("test.non-existent-key", 1), false);

  k = "serialized-key";
  Variant v1 = CREATE_VECTOR4(1, 2, 3, 4);
  kv = CREATE_MAP1(k, v1);
  mcc->t_set(k, v1);
  VS(mcc->t_get(kv), kv);

  Object v2 = Object(new c_stdclass());
  v2->o_set("name", -1, "value");
  kv = CREATE_MAP1(k, v2);
  mcc->t_set(k, v2);
  Variant r = mcc->t_get(kv);
  VS(r[k].toObject()->o_get("name", -1), "value");

  return Count(true);
}
