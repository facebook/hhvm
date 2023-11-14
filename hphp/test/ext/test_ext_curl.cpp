/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/test/ext/test_ext_curl.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/ext/curl/ext_curl.h"
#include "hphp/runtime/ext/std/ext_std_output.h"
#include "hphp/runtime/ext/zlib/ext_zlib.h"
#include "hphp/runtime/server/proxygen/proxygen-server.h"

#include <folly/Conv.h>

#include <string>

#define PORT_MIN 7100
#define PORT_MAX 7120

///////////////////////////////////////////////////////////////////////////////

struct TestCurlRequestHandler final : RequestHandler {
  explicit TestCurlRequestHandler(int timeout) : RequestHandler(timeout) {}
  // implementing RequestHandler
  void teardownRequest(Transport*) noexcept override {
    hphp_memory_cleanup();
  }

  void handleRequest(Transport* transport) override {
    g_context.getCheck();
    transport->addHeader("ECHOED", transport->getHeader("ECHO").c_str());

    if (transport->getMethod() == Transport::Method::POST) {
      size_t len = 0;
      const void *data = transport->getPostData(len);
      String res = "POST: ";
      res += String((char*)data, len, CopyString);
      transport->sendString(res.c_str());
    } else {
      transport->sendString("OK");
    }
  }
  void abortRequest(Transport* transport) override {
    transport->sendString("Aborted");
  }
};

static int s_server_port = 0;

static std::string get_request_uri() {
  return "http://localhost:" + folly::to<std::string>(s_server_port) +
    "/request";
}

static ServerPtr runServer() {
  for (s_server_port = PORT_MIN; s_server_port <= PORT_MAX; s_server_port++) {
    try {
      ServerPtr server = std::make_unique<ProxygenServer>(
        ServerOptions("127.0.0.1", s_server_port, 4));
      server->setRequestHandlerFactory<TestCurlRequestHandler>(0);
      server->start();
      return server;

    } catch (const FailedToListenException& e) {
      if (s_server_port == PORT_MAX) throw;
    }
  }
  return ServerPtr();
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtCurl::RunTests(const std::string &which) {
  bool ret = true;

  DECLARE_TEST_FUNCTIONS("function curl_write_func($s1, $s2) {"
                         " print 'curl_write_func called with ';"
                         " print $s2;"
                         " return strlen($s2);"
                         "}");

  ServerPtr server = runServer();

  RUN_TEST(test_curl_init);
  RUN_TEST(test_curl_version);
  RUN_TEST(test_curl_setopt);
  RUN_TEST(test_curl_setopt_array);
  RUN_TEST(test_curl_exec);
  RUN_TEST(test_curl_getinfo);
  RUN_TEST(test_curl_errno);
  RUN_TEST(test_curl_error);
  RUN_TEST(test_curl_close);
  RUN_TEST(test_curl_multi_init);
  RUN_TEST(test_curl_multi_add_handle);
  RUN_TEST(test_curl_multi_remove_handle);
  RUN_TEST(test_curl_multi_exec);
  RUN_TEST(test_curl_multi_select);
  RUN_TEST(test_curl_multi_getcontent);
  RUN_TEST(test_curl_multi_info_read);
  RUN_TEST(test_curl_multi_close);

  server->stop();

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtCurl::test_curl_init() {
  Variant c = HHVM_FN(curl_init)();
  VS(HHVM_FN(curl_errno)(c.toResource()), 0);
  VS(HHVM_FN(curl_error)(c.toResource()), "");
  return Count(true);
}

static const StaticString
  s_protocols("protocols"),
  s_url("url"),
  s_result("result"),
  s_code("code"),
  s_response("response"),
  s_headers("headers");

bool TestExtCurl::test_curl_version() {
  Variant ret = HHVM_FN(curl_version)();
  VERIFY(!ret.toArray()[s_protocols].toArray().empty());
  return Count(true);
}

bool TestExtCurl::test_curl_setopt() {
  Variant c = HHVM_FN(curl_init)();
  HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_URL,
                       String(get_request_uri()));
  HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_RETURNTRANSFER, true);
  Variant res = HHVM_FN(curl_exec)(c.toResource());
  VS(res, "OK");
  return Count(true);
}

bool TestExtCurl::test_curl_setopt_array() {
  Variant c = HHVM_FN(curl_init)();
  HHVM_FN(curl_setopt_array)
    (c.toResource(),
     make_dict_array(CURLOPT_URL, String(get_request_uri()),
                    CURLOPT_RETURNTRANSFER, true));
  Variant res = HHVM_FN(curl_exec)(c.toResource());
  VS(res, "OK");
  return Count(true);
}

bool TestExtCurl::test_curl_exec() {
  {
    Variant c = HHVM_FN(curl_init)(String(get_request_uri()));
    HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_RETURNTRANSFER, true);
    Variant res = HHVM_FN(curl_exec)(c.toResource());
    VS(res, "OK");
  }
  {
    Variant c = HHVM_FN(curl_init)(String(get_request_uri()));
    HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_WRITEFUNCTION,
                         "curl_write_func");
    HHVM_FN(ob_start)();
    HHVM_FN(curl_exec)(c.toResource());
    auto const res = HHVM_FN(ob_get_contents)().toString();
    VS(res, "curl_write_func called with OK");
    HHVM_FN(ob_end_clean)();
  }
  return Count(true);
}

bool TestExtCurl::test_curl_getinfo() {
  Variant c = HHVM_FN(curl_init)(String(get_request_uri()));
  HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_exec)(c.toResource());
  Variant ret = HHVM_FN(curl_getinfo)(c.toResource());
  VS(ret.toArray()[s_url], String(get_request_uri()));
  ret = HHVM_FN(curl_getinfo)(c.toResource(), CURLINFO_EFFECTIVE_URL);
  VS(ret, String(get_request_uri()));
  return Count(true);
}

bool TestExtCurl::test_curl_errno() {
  Variant c = HHVM_FN(curl_init)("http://www.thereisnosuchanurl");
  HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_exec)(c.toResource());
  Variant err = HHVM_FN(curl_errno)(c.toResource());
  VS(err, CURLE_COULDNT_RESOLVE_HOST);
  return Count(true);
}

bool TestExtCurl::test_curl_error() {
  Variant c = HHVM_FN(curl_init)("http://www.thereisnosuchanurl");
  HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_exec)(c.toResource());
  Variant err = HHVM_FN(curl_error)(c.toResource());
  VERIFY(equal(err, String("Couldn't resolve host 'www.thereisnosuchanurl'").get()) ||
         equal(err, String("Could not resolve host: www.thereisnosuchanurl"
                " (Domain name not found)").get()) ||
         equal(err, String("Could not resolve: www.thereisnosuchanurl"
                " (Domain name not found)").get()));
  return Count(true);
}

bool TestExtCurl::test_curl_close() {
  Variant c = HHVM_FN(curl_init)(String(get_request_uri()));
  HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_exec)(c.toResource());
  HHVM_FN(curl_close)(c.toResource());
  return Count(true);
}

bool TestExtCurl::test_curl_multi_init() {
  HHVM_FN(curl_multi_init)();
  return Count(true);
}

bool TestExtCurl::test_curl_multi_add_handle() {
  OptResource mh = HHVM_FN(curl_multi_init)();
  Variant c1 = HHVM_FN(curl_init)(String(get_request_uri()));
  Variant c2 = HHVM_FN(curl_init)(String(get_request_uri()));
  HHVM_FN(curl_multi_add_handle)(mh, c1.toResource());
  HHVM_FN(curl_multi_add_handle)(mh, c2.toResource());
  return Count(true);
}

bool TestExtCurl::test_curl_multi_remove_handle() {
  OptResource mh = HHVM_FN(curl_multi_init)();
  Variant c1 = HHVM_FN(curl_init)(String(get_request_uri()));
  Variant c2 = HHVM_FN(curl_init)(String(get_request_uri()));
  HHVM_FN(curl_multi_add_handle)(mh, c1.toResource());
  HHVM_FN(curl_multi_add_handle)(mh, c2.toResource());
  HHVM_FN(curl_multi_remove_handle)(mh, c1.toResource());
  return Count(true);
}

bool TestExtCurl::test_curl_multi_exec() {
  OptResource mh = HHVM_FN(curl_multi_init)();
  Variant c1 = HHVM_FN(curl_init)(String(get_request_uri()));
  Variant c2 = HHVM_FN(curl_init)(String(get_request_uri()));
  HHVM_FN(curl_setopt)(c1.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_setopt)(c2.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_multi_add_handle)(mh, c1.toResource());
  HHVM_FN(curl_multi_add_handle)(mh, c2.toResource());

  int64_t still_running;
  do {
    HHVM_FN(curl_multi_exec)(mh, still_running);
  } while (still_running > 0);

  return Count(true);
}

bool TestExtCurl::test_curl_multi_select() {
  OptResource mh = HHVM_FN(curl_multi_init)();
  Variant c1 = HHVM_FN(curl_init)(String(get_request_uri()));
  Variant c2 = HHVM_FN(curl_init)(String(get_request_uri()));
  HHVM_FN(curl_multi_add_handle)(mh, c1.toResource());
  HHVM_FN(curl_multi_add_handle)(mh, c2.toResource());
  VS(HHVM_FN(curl_multi_select)(mh), 0);
  return Count(true);
}

bool TestExtCurl::test_curl_multi_getcontent() {
  OptResource mh = HHVM_FN(curl_multi_init)();
  Variant c1 = HHVM_FN(curl_init)(String(get_request_uri()));
  Variant c2 = HHVM_FN(curl_init)(String(get_request_uri()));
  HHVM_FN(curl_setopt)(c1.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_setopt)(c2.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_multi_add_handle)(mh, c1.toResource());
  HHVM_FN(curl_multi_add_handle)(mh, c2.toResource());

  int64_t still_running;
  do {
    HHVM_FN(curl_multi_exec)(mh, still_running);
  } while (still_running > 0);

  VS(HHVM_FN(curl_multi_getcontent)(c1.toResource()), "OK");
  VS(HHVM_FN(curl_multi_getcontent)(c1.toResource()), "OK");
  VS(HHVM_FN(curl_multi_getcontent)(c2.toResource()), "OK");
  VS(HHVM_FN(curl_multi_getcontent)(c2.toResource()), "OK");
  return Count(true);
}

bool TestExtCurl::test_curl_multi_info_read() {
  OptResource mh = HHVM_FN(curl_multi_init)();
  Variant c1 = HHVM_FN(curl_init)(String(get_request_uri()));
  Variant c2 = HHVM_FN(curl_init)(String(get_request_uri()));
  HHVM_FN(curl_setopt)(c1.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_setopt)(c2.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_multi_add_handle)(mh, c1.toResource());
  HHVM_FN(curl_multi_add_handle)(mh, c2.toResource());

  int64_t still_running;
  do {
    HHVM_FN(curl_multi_exec)(mh, still_running);
  } while (still_running > 0);

  int64_t msgs_in_queue;
  Variant ret = HHVM_FN(curl_multi_info_read)(mh, msgs_in_queue);
  VS(ret.toArray()[s_result], 0);
  return Count(true);
}

bool TestExtCurl::test_curl_multi_close() {
  OptResource mh = HHVM_FN(curl_multi_init)();
  Variant c1 = HHVM_FN(curl_init)(String(get_request_uri()));
  Variant c2 = HHVM_FN(curl_init)(String(get_request_uri()));
  HHVM_FN(curl_setopt)(c1.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_setopt)(c2.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_multi_add_handle)(mh, c1.toResource());
  HHVM_FN(curl_multi_add_handle)(mh, c2.toResource());

  int64_t still_running;
  do {
    HHVM_FN(curl_multi_exec)(mh, still_running);
  } while (still_running > 0);

  HHVM_FN(curl_multi_close)(mh);
  return Count(true);
}
