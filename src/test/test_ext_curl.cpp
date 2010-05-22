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

#include <test/test_ext_curl.h>
#include <runtime/ext/ext_curl.h>
#include <runtime/ext/ext_output.h>
#include <runtime/ext/ext_zlib.h>
#include <runtime/base/server/libevent_server.h>

using namespace std;

#define REQUEST_URI "http://localhost:8080/request"

///////////////////////////////////////////////////////////////////////////////

class TestRequestHandler : public RequestHandler {
public:
  // implementing RequestHandler
  virtual void handleRequest(Transport *transport) {
    transport->addHeader("ECHOED", transport->getHeader("ECHO").c_str());

    if (transport->getMethod() == Transport::POST) {
      int len = 0;
      const void *data = transport->getPostData(len);
      String res = "POST: ";
      res += String((char*)data, len, CopyString);
      transport->sendString(res);
    } else {
      transport->sendString("OK");
    }
  }
};

static ServerPtr runServer() {
  for (int port = 8080; port <= 8088; port++) {
    try {
      ServerPtr server(new TypedServer<LibEventServer, TestRequestHandler>
                       ("127.0.0.1", port, 4, -1));
      server->start();
      return server;

    } catch (FailedToListenException e) {
      if (port == 8088) throw;
    }
  }
  return ServerPtr();
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtCurl::RunTests(const std::string &which) {
  bool ret = true;

  ServerPtr server = runServer();

  RUN_TEST(test_curl_init);
  RUN_TEST(test_curl_copy_handle);
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
  RUN_TEST(test_evhttp_set_cache);
  RUN_TEST(test_evhttp_get);
  RUN_TEST(test_evhttp_post);
  RUN_TEST(test_evhttp_post_gzip);
  RUN_TEST(test_evhttp_async_get);
  RUN_TEST(test_evhttp_async_post);
  RUN_TEST(test_evhttp_recv);

  server->stop();

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtCurl::test_curl_init() {
  Variant c = f_curl_init();
  VS(f_curl_errno(c), 0);
  VS(f_curl_error(c), "");
  return Count(true);
}

bool TestExtCurl::test_curl_copy_handle() {
  Variant c = f_curl_init();
  f_curl_setopt(c, k_CURLOPT_URL, REQUEST_URI);
  f_curl_setopt(c, k_CURLOPT_RETURNTRANSFER, true);
  Variant cpy = f_curl_copy_handle(c);
  f_curl_close(c); // to test cpy is still working fine
  Variant res = f_curl_exec(cpy);
  VS(res, "OK");
  return Count(true);
}

bool TestExtCurl::test_curl_version() {
  Variant ret = f_curl_version();
  VERIFY(!ret["protocols"].toArray().empty());
  return Count(true);
}

bool TestExtCurl::test_curl_setopt() {
  Variant c = f_curl_init();
  f_curl_setopt(c, k_CURLOPT_URL, REQUEST_URI);
  f_curl_setopt(c, k_CURLOPT_RETURNTRANSFER, true);
  Variant res = f_curl_exec(c);
  VS(res, "OK");
  return Count(true);
}

bool TestExtCurl::test_curl_setopt_array() {
  Variant c = f_curl_init();
  f_curl_setopt_array
    (c,
     CREATE_MAP2(k_CURLOPT_URL, REQUEST_URI,
                 k_CURLOPT_RETURNTRANSFER, true));
  Variant res = f_curl_exec(c);
  VS(res, "OK");
  return Count(true);
}

bool TestExtCurl::test_curl_exec() {
  {
    Variant c = f_curl_init(REQUEST_URI);
    f_curl_setopt(c, k_CURLOPT_RETURNTRANSFER, true);
    Variant res = f_curl_exec(c);
    VS(res, "OK");
  }
  {
    Variant c = f_curl_init(REQUEST_URI);
    f_curl_setopt(c, k_CURLOPT_WRITEFUNCTION, "curl_write_func");
    f_ob_start();
    f_curl_exec(c);
    String res = f_ob_get_contents();
    VS(res, "curl_write_func called with OK");
    f_ob_end_clean();
  }
  return Count(true);
}

bool TestExtCurl::test_curl_getinfo() {
  Variant c = f_curl_init(REQUEST_URI);
  f_curl_setopt(c, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_exec(c);
  Variant ret = f_curl_getinfo(c);
  VS(ret["url"], REQUEST_URI);
  ret = f_curl_getinfo(c, k_CURLINFO_EFFECTIVE_URL);
  VS(ret, REQUEST_URI);
  return Count(true);
}

bool TestExtCurl::test_curl_errno() {
  Variant c = f_curl_init("http://www.thereisnosuchanurl");
  f_curl_setopt(c, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_exec(c);
  Variant err = f_curl_errno(c);
  VS(err, k_CURLE_COULDNT_RESOLVE_HOST);
  return Count(true);
}

bool TestExtCurl::test_curl_error() {
  Variant c = f_curl_init("http://www.thereisnosuchanurl");
  f_curl_setopt(c, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_exec(c);
  Variant err = f_curl_error(c);
  VS(err, "Couldn't resolve host 'www.thereisnosuchanurl'");
  return Count(true);
}

bool TestExtCurl::test_curl_close() {
  Variant c = f_curl_init(REQUEST_URI);
  f_curl_setopt(c, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_exec(c);
  f_curl_close(c);
  return Count(true);
}

bool TestExtCurl::test_curl_multi_init() {
  f_curl_multi_init();
  return Count(true);
}

bool TestExtCurl::test_curl_multi_add_handle() {
  Object mh = f_curl_multi_init();
  Variant c1 = f_curl_init(REQUEST_URI);
  Variant c2 = f_curl_init(REQUEST_URI);
  f_curl_multi_add_handle(mh, c1);
  f_curl_multi_add_handle(mh, c2);
  return Count(true);
}

bool TestExtCurl::test_curl_multi_remove_handle() {
  Object mh = f_curl_multi_init();
  Variant c1 = f_curl_init(REQUEST_URI);
  Variant c2 = f_curl_init(REQUEST_URI);
  f_curl_multi_add_handle(mh, c1);
  f_curl_multi_add_handle(mh, c2);
  f_curl_multi_remove_handle(mh, c1);
  return Count(true);
}

bool TestExtCurl::test_curl_multi_exec() {
  Object mh = f_curl_multi_init();
  Variant c1 = f_curl_init(REQUEST_URI);
  Variant c2 = f_curl_init(REQUEST_URI);
  f_curl_setopt(c1, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_setopt(c2, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_multi_add_handle(mh, c1);
  f_curl_multi_add_handle(mh, c2);

  Variant still_running;
  do {
    f_curl_multi_exec(mh, ref(still_running));
  } while (more(still_running, 0));

  return Count(true);
}

bool TestExtCurl::test_curl_multi_select() {
  Object mh = f_curl_multi_init();
  Variant c1 = f_curl_init(REQUEST_URI);
  Variant c2 = f_curl_init(REQUEST_URI);
  f_curl_multi_add_handle(mh, c1);
  f_curl_multi_add_handle(mh, c2);
  VS(f_curl_multi_select(mh), 0);
  return Count(true);
}

bool TestExtCurl::test_curl_multi_getcontent() {
  Object mh = f_curl_multi_init();
  Variant c1 = f_curl_init(REQUEST_URI);
  Variant c2 = f_curl_init(REQUEST_URI);
  f_curl_setopt(c1, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_setopt(c2, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_multi_add_handle(mh, c1);
  f_curl_multi_add_handle(mh, c2);

  Variant still_running;
  do {
    f_curl_multi_exec(mh, ref(still_running));
  } while (more(still_running, 0));

  VS(f_curl_multi_getcontent(c1), "OK");
  VS(f_curl_multi_getcontent(c1), "OK");
  VS(f_curl_multi_getcontent(c2), "OK");
  VS(f_curl_multi_getcontent(c2), "OK");
  return Count(true);
}

bool TestExtCurl::test_curl_multi_info_read() {
  Object mh = f_curl_multi_init();
  Variant c1 = f_curl_init(REQUEST_URI);
  Variant c2 = f_curl_init(REQUEST_URI);
  f_curl_setopt(c1, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_setopt(c2, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_multi_add_handle(mh, c1);
  f_curl_multi_add_handle(mh, c2);

  Variant still_running;
  do {
    f_curl_multi_exec(mh, ref(still_running));
  } while (more(still_running, 0));

  Variant ret = f_curl_multi_info_read(mh);
  VS(ret["result"], 0);
  return Count(true);
}

bool TestExtCurl::test_curl_multi_close() {
  Object mh = f_curl_multi_init();
  Variant c1 = f_curl_init(REQUEST_URI);
  Variant c2 = f_curl_init(REQUEST_URI);
  f_curl_setopt(c1, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_setopt(c2, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_multi_add_handle(mh, c1);
  f_curl_multi_add_handle(mh, c2);

  Variant still_running;
  do {
    f_curl_multi_exec(mh, ref(still_running));
  } while (more(still_running, 0));

  f_curl_multi_close(mh);
  return Count(true);
}

bool TestExtCurl::test_evhttp_set_cache() {
  f_evhttp_set_cache("localhost", 4, 8080);
  for (int i = 0; i < 10; i++) {
    Variant ret = f_evhttp_get(REQUEST_URI, CREATE_VECTOR1("ECHO: foo"));
    VS(ret["code"], 200);
    VS(ret["response"], "OK");
    VS(ret["headers"][0], "ECHOED: foo");
    VS(ret["headers"][3], "Content-Length: 2");
  }

  return Count(true);
}

bool TestExtCurl::test_evhttp_get() {
  Variant ret = f_evhttp_get(REQUEST_URI, CREATE_VECTOR1("ECHO: foo"));
  VS(ret["code"], 200);
  VS(ret["response"], "OK");
  VS(ret["headers"][0], "ECHOED: foo");
  VS(ret["headers"][3], "Content-Length: 2");
  return Count(true);
}

bool TestExtCurl::test_evhttp_post() {
  Variant ret = f_evhttp_post(REQUEST_URI, "echo",
                              CREATE_VECTOR1("ECHO: foo"));
  VS(ret["code"], 200);
  VS(ret["response"], "POST: echo");
  VS(ret["headers"][0], "ECHOED: foo");
  VS(ret["headers"][3], "Content-Length: 10");
  return Count(true);
}

bool TestExtCurl::test_evhttp_post_gzip() {
  // we fill up 2k to avoid the "oh it's < 1000 bytes, forget compression"
  // logic in Transport's implementation.
  char fullPostBody[2048];
  memcpy(fullPostBody, "POST: ", 6);
  char* postBody = fullPostBody + 6;
  memset(postBody, 'a', sizeof(fullPostBody) - 7);
  fullPostBody[sizeof(fullPostBody) - 1] = '\0';
  Variant ret = f_evhttp_post(REQUEST_URI, postBody,
                              CREATE_VECTOR2("ECHO: foo",
                                             "Accept-Encoding: gzip"));
  VS(ret["code"], 200);
  VS(ret["response"], fullPostBody);
  VS(ret["headers"][0], "ECHOED: foo");
  VS(ret["headers"][1], "Content-Encoding: gzip");
  return Count(true);
}

bool TestExtCurl::test_evhttp_async_get() {
  Variant ret = f_evhttp_async_get(REQUEST_URI, CREATE_VECTOR1("ECHO: foo"));
  ret = f_evhttp_recv(ret);
  VS(ret["code"], 200);
  VS(ret["response"], "OK");
  VS(ret["headers"][0], "ECHOED: foo");
  VS(ret["headers"][3], "Content-Length: 2");
  return Count(true);
}

bool TestExtCurl::test_evhttp_async_post() {
  Variant ret = f_evhttp_async_post(REQUEST_URI, "echo",
                                    CREATE_VECTOR1("ECHO: foo"));
  ret = f_evhttp_recv(ret);
  VS(ret["code"], 200);
  VS(ret["response"], "POST: echo");
  VS(ret["headers"][0], "ECHOED: foo");
  VS(ret["headers"][3], "Content-Length: 10");
  return Count(true);
}

bool TestExtCurl::test_evhttp_recv() {
  // tested in test_evhttp_async_get() and test_evhttp_async_post()
  return Count(true);
}
