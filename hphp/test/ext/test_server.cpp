/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/test/ext/test_server.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/util/util.h"
#include "hphp/util/process.h"
#include "hphp/compiler/option.h"
#include "hphp/util/async-func.h"
#include "hphp/runtime/ext/ext_curl.h"
#include "hphp/runtime/ext/ext_options.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server.h"

#include <memory>

using namespace HPHP;

#define PORT_MIN 7300
#define PORT_MAX 7400

///////////////////////////////////////////////////////////////////////////////

TestServer::TestServer(const std::string serverType)
    : m_serverType(serverType) { }

static int s_server_port = 0;
static int s_admin_port = 0;
static int s_rpc_port = 0;
static int inherit_fd = -1;
static std::unique_ptr<AsyncFunc<TestServer>> s_func;
static char s_filename[MAXPATHLEN];

bool TestServer::VerifyServerResponse(const char *input, const char **outputs,
                                      const char **urls, int nUrls,
                                      const char *method,
                                      const char *header, const char *postdata,
                                      bool responseHeader,
                                      const char *file /* = "" */,
                                      int line /* = 0 */,
                                      int port /* = 0 */) {
  assert(input);
  if (port == 0) port = s_server_port;

  if (!CleanUp()) return false;
  string fullPath = "runtime/tmp/string";
  std::ofstream f(fullPath.c_str());
  if (!f) {
    printf("Unable to open %s for write. Run this test from hphp/.\n",
           fullPath.c_str());
    return false;
  }

  f << input;
  f.close();

  AsyncFunc<TestServer> func(this, &TestServer::RunServer);
  func.start();

  if (s_func) {
    s_func->waitForEnd();
    s_func.reset();
  }
  bool passed = true;

  string actual;

  int url = 0;
  for (url = 0; url < nUrls; url++) {
    String server = "http://";
    server += f_php_uname("n");
    server += ":" + lexical_cast<string>(port) + "/";
    server += urls[url];
    actual = "<No response from server>";
    string err;
    for (int i = 0; i < 10; i++) {
      Variant c = f_curl_init();
      f_curl_setopt(c.toResource(), k_CURLOPT_URL, server);
      f_curl_setopt(c.toResource(), k_CURLOPT_RETURNTRANSFER, true);
      if (postdata) {
        f_curl_setopt(c.toResource(), k_CURLOPT_POSTFIELDS, postdata);
        f_curl_setopt(c.toResource(), k_CURLOPT_POST, true);
      }
      if (header) {
        f_curl_setopt(c.toResource(), k_CURLOPT_HTTPHEADER,
                      make_packed_array(header));
      }
      if (responseHeader) {
        f_curl_setopt(c.toResource(), k_CURLOPT_HEADER, 1);
      }

      Variant res = f_curl_exec(c.toResource());
      if (!same(res, false)) {
        actual = (std::string) res.toString();
        break;
      }
      sleep(1); // wait until HTTP server is up and running
    }
    if (actual != outputs[url]) {
      if (!responseHeader ||
          actual.find(outputs[url]) == string::npos) {
        passed = false;
        break;
      }
    }
  }

  AsyncFunc<TestServer>(this, &TestServer::StopServer).run();
  func.waitForEnd();

  if (!passed) {
    printf("%s:%d\nParsing: [%s] (req %d)\nBet %d:\n"
           "--------------------------------------\n"
           "%s"
           "--------------------------------------\n"
           "Got %d:\n"
           "--------------------------------------\n"
           "%s"
           "--------------------------------------\n",
           file, line, input, url + 1, (int)strlen(outputs[url]), outputs[url],
           (int)actual.length(), actual.c_str());
    return false;
  }
  return true;
}

void TestServer::RunServer() {
  string out, err;
  string portConfig = "-vServer.Port=" + lexical_cast<string>(s_server_port);
  string adminConfig = "-vAdminServer.Port=" +
    lexical_cast<string>(s_admin_port);
  string rpcConfig = "-vSatellites.rpc.Port=" +
    lexical_cast<string>(s_rpc_port);
  string fd = lexical_cast<string>(inherit_fd);
  string option = (inherit_fd >= 0) ? (string("--port-fd=") + fd) :
    (string("-vServer.TakeoverFilename=") + string(s_filename));
  string serverType = string("-vServer.Type=") + m_serverType;

  const char *argv[] = {
    "", "--mode=server", "--config=test/ext/config-server.hdf",
    portConfig.c_str(), adminConfig.c_str(), rpcConfig.c_str(),
    option.c_str(),
    serverType.c_str(),
    NULL
  };

  if (Option::EnableEval < Option::FullEval) {
    argv[0] = "runtime/tmp/TestServer/test";
  } else {
    argv[0] = HHVM_PATH;
  }

  Process::Exec(argv[0], argv, NULL, out, &err);
}

void TestServer::StopServer() {
  for (int i = 0; i < 10; i++) {
    string out, err;
    Variant c = f_curl_init();
    String url = "http://";
    url += f_php_uname("n");
    url += ":" + lexical_cast<string>(s_admin_port) + "/stop";
    f_curl_setopt(c.toResource(), k_CURLOPT_URL, url);
    f_curl_setopt(c.toResource(), k_CURLOPT_RETURNTRANSFER, true);
    Variant res = f_curl_exec(c.toResource());
    if (!same(res, false)) {
      break;
    }
    sleep(1); // wait until HTTP server is up and running
  }
}

///////////////////////////////////////////////////////////////////////////////

class TestServerRequestHandler : public RequestHandler {
public:
  explicit TestServerRequestHandler(int timeout) : RequestHandler(timeout) {}
  // implementing RequestHandler
  virtual void handleRequest(Transport *transport) {
    // do nothing
  }
  virtual void abortRequest(Transport *transport) {
    // do nothing
  }
};

static int find_server_port(const std::string &serverType,
                            int port_min, int port_max) {
  for (int port = port_min; ; port++) {
    try {
      ServerPtr server = ServerFactoryRegistry::createServer(
        serverType, "127.0.0.1", port, 50);
      server->setRequestHandlerFactory<TestServerRequestHandler>(30);
      server->start();
      server->stop();
      server->waitForEnd();
      return port;
    } catch (const FailedToListenException& e) {
      if (port >= port_max) throw;
    }
  }
}

bool TestServer::RunTests(const std::string &which) {
  bool ret = true;

  {
    // TestSimpleServer finds a good port to listen on, so it must
    // always run.
    std::string which = "TestSimpleServer";
    RUN_TEST(TestSimpleServer);
  }
  s_admin_port = find_server_port(m_serverType, s_server_port + 1, PORT_MAX);
  s_rpc_port = find_server_port(m_serverType, s_admin_port + 1, PORT_MAX);

  RUN_TEST(TestInheritFdServer);
  RUN_TEST(TestTakeoverServer);
  RUN_TEST(TestSanity);
  RUN_TEST(TestServerVariables);
  RUN_TEST(TestInteraction);
  RUN_TEST(TestGet);
  RUN_TEST(TestPost);
  RUN_TEST(TestCookie);
  RUN_TEST(TestResponseHeader);
  RUN_TEST(TestSetCookie);
  //RUN_TEST(TestRequestHandling);
  RUN_TEST(TestHttpClient);
  RUN_TEST(TestRPCServer);
  RUN_TEST(TestXboxServer);
  RUN_TEST(TestPageletServer);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestServer::TestSanity() {
  VSR("<?php print 'Hello, World!';",
      "Hello, World!");
  return true;
}

bool TestServer::TestServerVariables() {
  VSR("<?php var_dump($_POST, $_GET);",
      "array(0) {\n}\narray(0) {\n}\n");

  VSR("<?php print $_SERVER['REQUEST_URI'];",
      "/string");

  VSGET("<?php "
        "var_dump($_SERVER['PATH_INFO']);"
        "var_dump(clean($_SERVER['PATH_TRANSLATED']));"
        "var_dump($_SERVER['SCRIPT_NAME']);"
        "var_dump($_SERVER['REQUEST_URI']);"
        "var_dump(clean($_SERVER['SCRIPT_FILENAME']));"
        "var_dump($_SERVER['QUERY_STRING']);"
        "function clean($x) { return str_replace(getcwd(),'',$x); }",

        "string(13) \"/path/subpath\"\n"
        "string(13) \"/path/subpath\"\n"
        "string(7) \"/string\"\n"
        "string(28) \"/string/path/subpath?a=1&b=2\"\n"
        "string(7) \"/string\"\n"
        "string(7) \"a=1&b=2\"\n",

        "string/path/subpath?a=1&b=2");

  VSGET("<?php "
        "var_dump($_SERVER['PATH_INFO']);"
        "var_dump(clean($_SERVER['PATH_TRANSLATED']));"
        "var_dump($_SERVER['SCRIPT_NAME']);"
        "var_dump($_SERVER['REQUEST_URI']);"
        "var_dump(clean($_SERVER['SCRIPT_FILENAME']));"
        "var_dump($_SERVER['QUERY_STRING']);"
        "var_dump(isset($_ENV['HPHP_RPC']));"
        "function clean($x) { return str_replace(getcwd(),'',$x); }",

        "NULL\n"
        "string(7) \"/string\"\n"
        "string(7) \"/string\"\n"
        "string(15) \"/string?a=1&b=2\"\n"
        "string(7) \"/string\"\n"
        "string(7) \"a=1&b=2\"\n"
        "bool(false)\n",

        "string?a=1&b=2");

  return true;
}

bool TestServer::TestInteraction() {
  // run this twice to test lvalBlackHole
  VSR2("<?php "
        "$a[] = new stdclass;"
        "var_dump(count(array_combine($a, $a)));",
        "int(0)\n");

  return true;
}

bool TestServer::TestGet() {
  VSGET("<?php var_dump($_GET['name']);",
        "string(0) \"\"\n", "string?name");

  VSGET("<?php var_dump($_GET['name'], $_GET['id']);",
        "string(0) \"\"\nstring(1) \"1\"\n", "string?name&id=1");

  VSGET("<?php print $_GET['name'];",
        "value", "string?name=value");

  VSGET("<?php var_dump($_GET['names']);",
        "array(2) {\n"
        "  [1]=>\n"
        "  string(3) \"foo\"\n"
        "  [2]=>\n"
        "  string(3) \"bar\"\n"
        "}\n",
        "string?names[1]=foo&names[2]=bar");

  VSGET("<?php var_dump($_GET['names']);",
        "array(2) {\n"
        "  [0]=>\n"
        "  string(3) \"foo\"\n"
        "  [1]=>\n"
        "  string(3) \"bar\"\n"
        "}\n",
        "string?names[]=foo&names[]=bar");

  VSGET("<?php print $_REQUEST['name'];",
        "value", "string?name=value");

  return true;
}

bool TestServer::TestPost() {
  const char *params = "name=value";

  VSPOST("<?php print $_POST['name'];",
         "value", "string", params);

  VSPOST("<?php print $_REQUEST['name'];",
         "value", "string", params);

  VSPOST("<?php print $HTTP_RAW_POST_DATA;",
         "name=value", "string", params);

  return true;
}

bool TestServer::TestCookie() {
  VSRX("<?php print $_COOKIE['name'];",
       "value", "string", "GET", "Cookie: name=value;", nullptr);

  VSRX("<?php print $_COOKIE['name2'];",
       "value2", "string", "GET", "Cookie: n=v;name2=value2;n3=v3", nullptr);

  return true;
}

bool TestServer::TestResponseHeader() {
  VSR("<?php header('Set-Cookie: name=value'); var_dump(headers_list());",
      "array(1) {\n"
      "  [0]=>\n"
      "  string(22) \"Set-Cookie: name=value\"\n"
      "}\n");

  VSRES("<?php header('Set-Cookie: name=value');",
        "Set-Cookie: name=value");

  VSRES("<?php header('Location: new/url');",
        "302");

  VSRES("<?php header(\"Test-Header: x\ry\"); echo 'done';",
        "done");

  return true;
}

bool TestServer::TestSetCookie() {
  VSR("<?php setcookie('name', 'value'); var_dump(headers_list());",
      "array(1) {\n"
      "  [0]=>\n"
      "  string(22) \"Set-Cookie: name=value\"\n"
      "}\n");
  return true;

  VSRES("<?php setcookie('name', 'value');",
        "Set-Cookie: name=value");

  return true;
}

///////////////////////////////////////////////////////////////////////////////

class TestTransport : public Transport {
public:
  TestTransport() : m_code(0) {}

  int m_code;
  std::string m_response;

  /**
   * Implementing HttpTransport...
   */
  virtual const char *getUrl() { return "/string";}
  virtual const char *getRemoteHost() { return "remote";}
  virtual const void *getPostData(int &size) { size = 0; return nullptr;}
  virtual uint16_t getRemotePort() { return 0; }
  virtual Method getMethod() { return Transport::Method::GET;}
  virtual std::string getHeader(const char *name) { return "";}
  virtual void getHeaders(HeaderMap &headers) {}
  virtual void addHeaderImpl(const char *name, const char *value) {}
  virtual void removeHeaderImpl(const char *name) {}

  virtual void sendImpl(const void *data, int size, int code, bool chunked) {
    m_response.clear();
    m_response.append((const char *)data, size);
    m_code = code;
  }

  void process() {
    HttpRequestHandler handler(0);
    for (unsigned int i = 0; i < 100; i++) {
      handler.handleRequest(this);
    }
  }
};

typedef std::shared_ptr<TestTransport> TestTransportPtr;
typedef std::vector<TestTransportPtr> TestTransportPtrVec;
typedef AsyncFunc<TestTransport> TestTransportAsyncFunc;
typedef std::shared_ptr<TestTransportAsyncFunc> TestTransportAsyncFuncPtr;
typedef std::vector<TestTransportAsyncFuncPtr> TestTransportAsyncFuncPtrVec;

#define TEST_SIZE 100

/**
 * Start processing TEST_SIZE number of requests at the same time with
 * that many threads. This is mainly testing global variables to make sure
 * all handling are thread-safe.
 */
bool TestServer::TestRequestHandling() {
  RuntimeOption::AllowedFiles.insert("/string");
  TestTransportPtrVec transports(TEST_SIZE);
  TestTransportAsyncFuncPtrVec funcs(TEST_SIZE);
  for (unsigned int i = 0; i < TEST_SIZE; i++) {
    TestTransport *transport = new TestTransport();
    transports[i] = TestTransportPtr(transport);
    funcs[i] = TestTransportAsyncFuncPtr
      (new TestTransportAsyncFunc(transport, &TestTransport::process));
  }

  for (unsigned int i = 0; i < TEST_SIZE; i++) {
    funcs[i]->start();
  }
  for (unsigned int i = 0; i < TEST_SIZE; i++) {
    funcs[i]->waitForEnd();
  }
  for (unsigned int i = 0; i < TEST_SIZE; i++) {
    VS(transports[i]->m_code, 200);
    VS(String(transports[i]->m_response), "Hello, world!");
  }
  return Count(true);
}

bool TestServer::TestSimpleServer() {
  s_server_port = find_server_port(m_serverType, PORT_MIN, PORT_MAX);
  return Count(true);
}

static bool PreBindSocketHelper(struct addrinfo *info) {
  if (info->ai_family != AF_INET && info->ai_family != AF_INET6) {
    printf("No IPV4/6 interface found.\n");
    return false;
  }

  int fd = socket(info->ai_family, SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0) {
    printf("Error creating socket: %s\n", strerror(errno));
    return false;
  }

  int ret = ::bind(fd, info->ai_addr, info->ai_addrlen);
  if (ret < 0) {
    printf("Error binding socket to port %d: %s\n", s_server_port,
        strerror(errno));
    return false;
  }

  inherit_fd = fd;
  return true;
}

bool TestServer::PreBindSocket() {
  struct addrinfo hints, *res, *res0;
  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

  if (getaddrinfo(nullptr, lexical_cast<string>(s_server_port).c_str(),
                  &hints, &res0) < 0) {
    printf("Error in getaddrinfo(): %s\n", strerror(errno));
    return false;
  }

  for (res = res0; res; res = res->ai_next) {
    if (res->ai_family == AF_INET6 || res->ai_next == nullptr) {
      break;
    }
  }

  bool ret = PreBindSocketHelper(res);
  freeaddrinfo(res0);
  return ret;
}

void TestServer::CleanupPreBoundSocket() {
  close(inherit_fd);
  inherit_fd = -1;
}

bool TestServer::TestInheritFdServer() {
  WITH_PREBOUND_SOCKET(VSR("<?php print 'Hello, World!';",
      "Hello, World!"));
  return true;
}

bool TestServer::TestTakeoverServer() {
  // start a server
  snprintf(s_filename, MAXPATHLEN, "/tmp/hphp_takeover_XXXXXX");
  int tmpfd = mkstemp(s_filename);
  close(tmpfd);

  s_func.reset(new AsyncFunc<TestServer>(this, &TestServer::RunServer));
  s_func->start();

  // Wait for the server to actually start
  HttpClient http;
  StringBuffer response;
  vector<String> responseHeaders;
  string url = "http://127.0.0.1:" + lexical_cast<string>(s_server_port) +
    "/status.php";
  HeaderMap headers;
  for (int i = 0; i < 10; i++) {
    int code = http.get(url.c_str(), response, &headers, &responseHeaders);
    if (code > 0) {
      break;
    }
    sleep(1);
  }

  // will start a second server, which should takeover
  VSR("<?php print 'Hello, World!';",
      "Hello, World!");
  unlink(s_filename);
  s_filename[0] = 0;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

class EchoHandler : public RequestHandler {
public:
  explicit EchoHandler(int timeout) : RequestHandler(timeout) {}
  // implementing RequestHandler
  virtual void handleRequest(Transport *transport) {
    HeaderMap headers;
    transport->getHeaders(headers);

    string response;
    response = "\nGET param: name = ";
    response += transport->getParam("name");

    if (transport->getMethod() == Transport::Method::POST) {
      int size = 0;
      const char *data = (const char *)transport->getPostData(size);
      response += "\nPOST data: ";
      response += string(data, size);
    }

    for (HeaderMap::const_iterator iter = headers.begin();
         iter != headers.end(); ++iter) {
      response += "\nHeader: ";
      response += iter->first;
      for (unsigned int i = 0; i < iter->second.size(); i++) {
        response += "\n";
        response += lexical_cast<string>(i);
        response += ": ";
        response += iter->second[i];
      }
    }

    transport->addHeader("Custom", "blah");
    transport->sendString(response);
  }
  virtual void abortRequest(Transport *transport) {
    transport->sendString("Service Unavailable", 503);
  }
};

bool TestServer::TestHttpClient() {
  ServerPtr server;
  for (s_server_port = PORT_MIN; s_server_port <= PORT_MAX; s_server_port++) {
    try {
      server = ServerFactoryRegistry::createServer(
        m_serverType, "127.0.0.1", s_server_port, 50);
      server->setRequestHandlerFactory<EchoHandler>(0);
      server->start();
      break;
    } catch (const FailedToListenException& e) {
      if (s_server_port == PORT_MAX) throw;
    }
  }

  HeaderMap headers;
  headers["Cookie"].push_back("c1=v1;c2=v2;");
  headers["Cookie"].push_back("c3=v3;c4=v4;");
  string url = "http://127.0.0.1:" + lexical_cast<string>(s_server_port) +
    "/echo?name=value";

  static const StaticString s_Custom_colon_blah("Custom: blah");

  for (int i = 0; i < 10; i++) {
    HttpClient http;
    StringBuffer response;
    vector<String> responseHeaders;
    int code = http.get(url.c_str(), response, &headers, &responseHeaders);
    VS(code, 200);
    VS(response.data(),
       ("\nGET param: name = value"
        "\nHeader: Accept"
        "\n0: */*"
        "\nHeader: Cookie"
        "\n0: c1=v1;c2=v2;"
        "\n1: c3=v3;c4=v4;"
        "\nHeader: Host"
        "\n0: 127.0.0.1:" + lexical_cast<string>(s_server_port)).c_str());

    bool found = false;
    for (unsigned int i = 0; i < responseHeaders.size(); i++) {
      if (responseHeaders[i] == s_Custom_colon_blah) {
        found = true;
      }
    }
    VERIFY(found);
  }
  for (int i = 0; i < 10; i++) {
    HttpClient http;
    StringBuffer response;
    vector<String> responseHeaders;
    int code = http.post(url.c_str(), "postdata", 8, response, &headers,
                         &responseHeaders);
    VS(code, 200);
    VS(response.data(),
       ("\nGET param: name = value"
        "\nPOST data: postdata"
        "\nHeader: Accept"
        "\n0: */*"
        "\nHeader: Content-Length"
        "\n0: 8"
        "\nHeader: Content-Type"
        "\n0: application/x-www-form-urlencoded"
        "\nHeader: Cookie"
        "\n0: c1=v1;c2=v2;"
        "\n1: c3=v3;c4=v4;"
        "\nHeader: Host"
        "\n0: 127.0.0.1:" + lexical_cast<string>(s_server_port)).c_str());

    bool found = false;
    for (unsigned int i = 0; i < responseHeaders.size(); i++) {
      if (responseHeaders[i] == s_Custom_colon_blah) {
        found = true;
      }
    }
    VERIFY(found);
  }

  server->stop();
  server->waitForEnd();
  return Count(true);
}

bool TestServer::TestRPCServer() {
  // the simplest case
  VSGETP("<?php\n"
         "function f() { return 100; }\n",
         "100",
         "f?auth=test",
         s_rpc_port);

  // array output
  VSGETP("<?php\n"
         "function f($a) { return array(1, 2, 3, $a); }\n",
         "[1,2,3,\"hello\"]",
         "f?auth=test&p=\"hello\"",
         s_rpc_port);

  // associate arrays
  VSGETP("<?php\n"
         "function f($a, $b) { return array_merge($a, $b); }\n",
         "{\"a\":1,\"0\":2,\"1\":1,\"2\":2}",
         "f?auth=test&p={\"a\":1,\"1\":2}&p=[1,2]",
         s_rpc_port);

  // builtin function and static method
  VSGETP("<?php\n"
         "class A { static function f($a) { return $a; } }\n",
         "100",
         "call_user_func?auth=test&p=\"A::f\"&p=100",
         s_rpc_port);

  // invoking a file, with NO json encoding
  // "int(100)" is printed twice, one from warmup, and the other from include
  VSGETP("<?php\n"
         "var_dump(100);\n",
         "int(100)\n"
         "int(100)\n",
         "?include=string&output=1&auth=test",
         s_rpc_port);

  VSGETP("<?php\n"
         "var_dump(isset($_ENV['HPHP_RPC']));\n",
         "bool(true)\n"
         "bool(true)\n",
         "?include=string&output=1&auth=test",
         s_rpc_port);

  return true;
}

bool TestServer::TestXboxServer() {
  VSGET("<?php\n"
        "if (array_key_exists('main', $_GET)) {\n"
        "  $t = xbox_task_start('1');\n"
        "  xbox_task_result($t, 0, $r);\n"
        "  var_dump($r);\n"
        "  $t = xbox_task_start('2');\n"
        "  xbox_task_result($t, 0, $r);\n"
        "  $t = xbox_task_start('1');\n"
        "  xbox_task_result($t, 0, $r);\n"
        "  var_dump($r);\n"
        "  sleep(7);\n"
        "  $t = xbox_task_start('3');\n"
        "  xbox_task_result($t, 0, $r);\n"
        "  var_dump($r);\n"
        "  sleep(2);\n"
        "  $t = xbox_task_start('4');\n"
        "  xbox_task_result($t, 0, $r);\n"
        "  $t = xbox_task_start('3');\n"
        "  xbox_task_result($t, 0, $r);\n"
        "  var_dump($r);\n"
        "  sleep(2);\n"
        "  $t = xbox_task_start('3');\n"
        "  xbox_task_result($t, 0, $r);\n"
        "  var_dump($r);\n"
        "} else {\n"
        "  function xbox_process_message($msg) {\n"
        "    if ($msg == '1') return xbox_get_thread_timeout();\n"
        "    else if ($msg == '2') xbox_set_thread_timeout(5);\n"
        "    else if ($msg == '3') return xbox_get_thread_time();\n"
        "    else xbox_schedule_thread_reset();\n"
        "  }\n"
        "}\n",
        "int(10)\n"
        "int(5)\n"
        "int(0)\n"
        "int(0)\n"
        "int(0)\n",
        "string?main=1");

  return true;
}

bool TestServer::TestPageletServer() {
  VSGET("<?php\n"
        "if (array_key_exists('pagelet', $_GET)) {\n"
        "  echo 'Hello from the pagelet!';\n"
        "} else {\n"
        "  $h = array('Host: ' . $_SERVER['HTTP_HOST']);\n"
        "  $t = pagelet_server_task_start('/string?pagelet=1', $h, '');\n"
        "  echo 'First! ';\n"
        "  $r = pagelet_server_task_result($t, $h, $c);\n"
        "  echo $r;\n"
        "}\n",
        "First! Hello from the pagelet!",
        "string");

  // POST vs GET
  VSGET("<?php\n"
        "if (array_key_exists('pagelet', $_GET)) {\n"
        "  echo $_SERVER['REQUEST_METHOD'];\n"
        "} else {\n"
        "  $h = array('Host: ' . $_SERVER['HTTP_HOST']);\n"
        "  $t = pagelet_server_task_start('/string?pagelet=1', $h, '');\n"
        "  echo 'First! ';\n"
        "  $r = pagelet_server_task_result($t, $h, $c);\n"
        "  echo $r;\n"
        "}\n",
        "First! GET",
        "string");

  VSGET("<?php\n"
        "if ($_SERVER['THREAD_TYPE'] == 'Pagelet Thread') {\n"
        "  echo 'hello';\n"
        "  pagelet_server_flush();\n"
        "  ob_start();\n"
        "  echo 'world';\n"
        "  pagelet_server_flush();\n"
        "  echo 'what';\n"
        "} else {\n"
        "  $h = array('Host: ' . $_SERVER['HTTP_HOST']);\n"
        "  $t = pagelet_server_task_start('/string', $h, '');\n"
        "  for ($i = 0; ; $i++) {\n"
        "    while (($s = pagelet_server_task_status($t)) == \n"
        "           PAGELET_NOT_READY) { sleep(1); }\n"
        "    echo \"Step $i:\\n\";\n"
        "    $r = pagelet_server_task_result($t, $h, $c);\n"
        "    echo $r . \"\\n\";\n"
        "    if ($s == PAGELET_DONE) break;\n"
        "  }\n"
        "}\n",
        "Step 0:\n"
        "hello\n"
        "Step 1:\n"
        "world\n"
        "Step 2:\n"
        "what\n",
        "string");

  return true;
}
