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

#include "hphp/test/ext/test_server.h"

#include "hphp/compiler/option.h"

#include "hphp/util/async-func.h"
#include "hphp/util/process-exec.h"

#include "hphp/runtime/ext/curl/ext_curl.h"
#include "hphp/runtime/ext/std/ext_std_options.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server.h"

#include <sys/types.h>

#include <fstream>
#include <memory>
#include <sys/param.h>
#include <vector>

#include <folly/Conv.h>
#include <folly/portability/Sockets.h>

using namespace HPHP;

#define PORT_MIN 1024
#define PORT_MAX 65535

///////////////////////////////////////////////////////////////////////////////

TestServer::TestServer(const std::string serverType)
    : m_serverType(serverType) { }

static int s_server_port = 0;
static int s_admin_port = 0;
static int s_rpc_port = 0;
static int inherit_fd = -1;
static std::unique_ptr<AsyncFunc<TestServer>> s_func;
static char s_pidfile[PATH_MAX];
static char s_repoFile[PATH_MAX];
static char s_logFile[PATH_MAX];
static char s_filename[PATH_MAX];
static int k_timeout = 30;

bool TestServer::VerifyServerResponse(const char* input, const char** outputs,
                                      const char** urls, int nUrls,
                                      const char* /*method*/,
                                      const char* header, const char* postdata,
                                      bool responseHeader,
                                      const char* file /* = "" */,
                                      int line /* = 0 */, int port /* = 0 */) {
  assert(input);
  if (port == 0) port = s_server_port;

  std::string fullPath = "runtime/tmp/string";
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

  bool passed = true;
  if (s_func) {
    if (!s_func->waitForEnd(k_timeout)) {
      // Takeover didn't complete in 30s, stop the old server
      fprintf(stderr, "stopping HHVM\n");
      AsyncFunc<TestServer> stopFunc(this, &TestServer::KillServer);
      stopFunc.run();
      fprintf(stderr, "Waiting for stop\n");
      stopFunc.waitForEnd();
      fprintf(stderr, "Waiting for old HHVM\n");
      s_func->waitForEnd();
      // Mark this test a failure
      fprintf(stderr, "Proceeding to test\n");
      passed = false;
    }
    s_func.reset();
  }

  std::string actual;

  int url = 0;
  for (url = 0; url < nUrls; url++) {
    String server = "http://";
    server += HHVM_FN(php_uname)("n").toString();
    server += ":" + folly::to<std::string>(port) + "/";
    server += urls[url];
    actual = "<No response from server>";
    std::string err;
    for (int i = 0; i < 50; i++) {
      Variant c = HHVM_FN(curl_init)();
      HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_URL, server);
      HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_RETURNTRANSFER, true);
      if (postdata) {
        HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_POSTFIELDS, postdata);
        HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_POST, true);
      }
      if (header) {
        HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_HTTPHEADER,
                      make_varray(header));
      }
      if (responseHeader) {
        HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_HEADER, 1);
      }

      Variant res = HHVM_FN(curl_exec)(c.toResource());
      if (!same(res, false)) {
        actual = res.toString().toCppString();
        break;
      }
      sleep(1); // wait until HTTP server is up and running
    }
    if (actual != outputs[url]) {
      if (!responseHeader ||
          actual.find(outputs[url]) == std::string::npos) {
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
  std::string out, err;
  auto const portConfig = "-vServer.Port=" +
    folly::to<std::string>(s_server_port);
  auto const adminConfig = "-vAdminServer.Port=" +
    folly::to<std::string>(s_admin_port);
  auto const rpcConfig = "-vSatellites.rpc.Port=" +
    folly::to<std::string>(s_rpc_port);
  auto const fd = folly::to<std::string>(inherit_fd);
  auto option = inherit_fd >= 0
    ? "--port-fd=" + fd
    : "-vServer.TakeoverFilename=" + std::string(s_filename);
  auto serverType = std::string("-vServer.Type=") + m_serverType;
  auto pidFile = std::string("-vPidFile=") + s_pidfile;
  auto repoFile = std::string("-vRepo.Central.Path=") + s_repoFile;
  auto logFile = std::string("-vLog.File=") + s_logFile;

  const char *argv[] = {
    "__HHVM__", "--mode=server", "--config=test/ext/config-server.hdf",
    portConfig.c_str(), adminConfig.c_str(), rpcConfig.c_str(),
    option.c_str(), serverType.c_str(), pidFile.c_str(), repoFile.c_str(),
    logFile.c_str(),
    nullptr
  };

#ifdef HHVM_PATH
  // replace __HHVM__
  argv[0] = HHVM_PATH;
#endif

  HPHP::proc::exec(argv[0], argv, nullptr, out, &err);
}

void TestServer::StopServer() {
  for (int i = 0; i < 10; i++) {
    Variant c = HHVM_FN(curl_init)();
    String url = "http://";
    url += HHVM_FN(php_uname)("n").toString();
    url += ":" + folly::to<std::string>(s_admin_port) + "/stop";
    HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_URL, url);
    HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_RETURNTRANSFER, true);
    HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_TIMEOUT, 1);
    HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_CONNECTTIMEOUT, 1);
    Variant res = HHVM_FN(curl_exec)(c.toResource());
    if (!same(res, false)) {
      return;
    }
    sleep(1); // wait until HTTP server is up and running
  }
  KillServer();
}

void TestServer::KillServer() {
  fprintf(stderr, "Have to kill HHVM\n");
  // Getting more aggresive
  char buf[1024];
  int fd = open(s_pidfile, O_RDONLY);
  int ret = read(fd, buf, sizeof(buf) - 1);
  if (ret <= 0) {
    printf("Can't read pid from pid file %s\n", s_pidfile);
    return;
  }
  buf[ret] = 0;
  std::string out, err;
  const char *argv[] = {"kill", buf, nullptr};
  for (int i = 0; i < 10; i++) {
    auto ret = HPHP::proc::exec(argv[0], argv, nullptr, out, &err);
    if (ret) {
      return;
    }
  }

  // Last resort
  const char *argv9[] = {"kill", "-9", buf, nullptr};
  for (int i = 0; i < 10; i++) {
    auto ret = HPHP::proc::exec(argv9[0], argv9, nullptr, out, &err);
    if (ret) {
      return;
    }
  }

  printf("Can't kill pid %s read from pid file %s\n", buf, s_pidfile);
}

///////////////////////////////////////////////////////////////////////////////

struct TestServerRequestHandler : RequestHandler {
  explicit TestServerRequestHandler(int timeout) : RequestHandler(timeout) {}
  // implementing RequestHandler
  void handleRequest(Transport* /*transport*/) override {
    // do nothing
  }
  void abortRequest(Transport* /*transport*/) override {
    // do nothing
  }
};

static int find_server_port(const std::string &serverType) {
  for (int tries = 0; true; tries++) {
    auto port = (rand() % (PORT_MAX - PORT_MIN)) + PORT_MIN;
    try {
      ServerPtr server = ServerFactoryRegistry::createServer(
        serverType, "127.0.0.1", port, 50);
      server->setRequestHandlerFactory<TestServerRequestHandler>(k_timeout);
      server->start();
      server->stop();
      server->waitForEnd();
      return port;
    } catch (const FailedToListenException& e) {
      if (tries >= 100) throw;
    }
  }
}

bool TestServer::RunTests(const std::string &which) {
  bool ret = true;

  srand(time(0));
  s_server_port = find_server_port(m_serverType);
  s_admin_port = find_server_port(m_serverType);
  s_rpc_port = find_server_port(m_serverType);
  snprintf(s_pidfile, MAXPATHLEN, "/tmp/test_server.hhvm.pid_XXXXXX");
  int tmpfd = mkstemp(s_pidfile);
  close(tmpfd);
  snprintf(s_repoFile, MAXPATHLEN, "/tmp/test_server.hhvm.hhbc_XXXXXX");
  tmpfd = mkstemp(s_repoFile);
  close(tmpfd);
  snprintf(s_logFile, MAXPATHLEN, "/tmp/test_server.hhvm.log_XXXXXX");
  tmpfd = mkstemp(s_logFile);
  close(tmpfd);

  RUN_TEST(TestInheritFdServer);
  RUN_TEST(TestTakeoverServer);
  RUN_TEST(TestSanity);
  RUN_TEST(TestServerVariables);
  RUN_TEST(TestInteraction);
  RUN_TEST(TestGet);
  RUN_TEST(TestPost);
  RUN_TEST(TestExpectContinue);
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
  VSR("<?hh print 'Hello, World!';",
      "Hello, World!");
  return true;
}

bool TestServer::TestServerVariables() {
  VSR("<?hh var_dump($_POST, $_GET);",
      "array(0) {\n}\narray(0) {\n}\n");

  VSR("<?hh print $_SERVER['REQUEST_URI'];",
      "/string");

  VSGET("<?hh "
        "var_dump($_SERVER['PATH_INFO']);"
        "var_dump(clean($_SERVER['PATH_TRANSLATED']));"
        "var_dump($_SERVER['SCRIPT_NAME']);"
        "var_dump($_SERVER['REQUEST_URI']);"
        "var_dump(clean($_SERVER['SCRIPT_FILENAME']));"
        "var_dump($_SERVER['QUERY_STRING']);"
        "function clean($x) { return str_replace(getcwd(),'',$x); }",

        "string(13) \"/path/subpath\"\n"
        "string(20) \"/string/path/subpath\"\n"
        "string(7) \"/string\"\n"
        "string(28) \"/string/path/subpath?a=1&b=2\"\n"
        "string(7) \"/string\"\n"
        "string(7) \"a=1&b=2\"\n",

        "string/path/subpath?a=1&b=2");

  VSGET("<?hh "
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
  VSR2("<?hh "
        "$a[] = new stdclass;"
        "var_dump(count(array_combine($a, $a)));",
        "");

  return true;
}

bool TestServer::TestGet() {
  VSGET("<?hh var_dump($_GET['name']);",
        "string(0) \"\"\n", "string?name");

  VSGET("<?hh var_dump($_GET['name'], $_GET['id']);",
        "string(0) \"\"\nstring(1) \"1\"\n", "string?name&id=1");

  VSGET("<?hh print $_GET['name'];",
        "value", "string?name=value");

  VSGET("<?hh var_dump($_GET['names']);",
        "array(2) {\n"
        "  [1]=>\n"
        "  string(3) \"foo\"\n"
        "  [2]=>\n"
        "  string(3) \"bar\"\n"
        "}\n",
        "string?names[1]=foo&names[2]=bar");

  VSGET("<?hh var_dump($_GET['names']);",
        "array(2) {\n"
        "  [0]=>\n"
        "  string(3) \"foo\"\n"
        "  [1]=>\n"
        "  string(3) \"bar\"\n"
        "}\n",
        "string?names[]=foo&names[]=bar");

  VSGET("<?hh print $_REQUEST['name'];",
        "value", "string?name=value");

  return true;
}

bool TestServer::TestPost() {
  const char *params = "name=value";

  VSPOST("<?hh print $_POST['name'];",
         "value", "string", params);

  VSPOST("<?hh print $_REQUEST['name'];",
         "value", "string", params);

  VSPOST("<?hh print $HTTP_RAW_POST_DATA;",
         "name=value", "string", params);

  return true;
}

bool TestServer::TestExpectContinue() {
  const char *params = "name=value";

  VSRX("<?hh print $_POST['name'];",
       "value", "string", "POST", "Expect: 100-continue", params);

  return true;
}

bool TestServer::TestCookie() {
  VSRX("<?hh print $_COOKIE['name'];",
       "value", "string", "GET", "Cookie: name=value;", nullptr);

  VSRX("<?hh print $_COOKIE['name2'];",
       "value2", "string", "GET", "Cookie: n=v;name2=value2;n3=v3", nullptr);

  return true;
}

bool TestServer::TestResponseHeader() {
  VSR("<?hh header('Set-Cookie: name=value'); var_dump(headers_list());",
      "array(1) {\n"
      "  [0]=>\n"
      "  string(22) \"Set-Cookie: name=value\"\n"
      "}\n");

  VSRES("<?hh header('Set-Cookie: name=value');",
        "Set-Cookie: name=value");

  VSRES("<?hh header('Location: new/url');",
        "302");

  VSRES("<?hh header(\"Test-Header: x\ry\"); echo 'done';",
        "done");

  return true;
}

bool TestServer::TestSetCookie() {
  VSR("<?hh setcookie('name', 'value'); var_dump(headers_list());",
      "array(1) {\n"
      "  [0]=>\n"
      "  string(22) \"Set-Cookie: name=value\"\n"
      "}\n");
  return true;

  VSRES("<?hh setcookie('name', 'value');",
        "Set-Cookie: name=value");

  return true;
}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_test("test");

struct TestTransport final : Transport {
  TestTransport() : m_code(0) {}

  int m_code;
  std::string m_response;

  /**
   * Implementing HttpTransport...
   */
  const char *getUrl() override { return "/string"; }
  const char *getRemoteHost() override { return "remote"; }
  const void *getPostData(size_t &size) override { size = 0; return nullptr; }
  uint16_t getRemotePort() override { return 0; }
  Method getMethod() override { return Transport::Method::GET; }
  std::string getHeader(const char* /*name*/) override { return ""; }
  const HeaderMap& getHeaders() override {
    static const HeaderMap emptyMap{};
    return emptyMap;
  }
  void addHeaderImpl(const char* /*name*/, const char* /*value*/) override {}
  void removeHeaderImpl(const char* /*name*/) override {}

  /**
   * Get a description of the type of transport.
   */
  String describe() const override {
    return s_test;
  }

  void sendImpl(const void* data, int size, int code, bool /*chunked*/,
                bool /*eom*/) override {
    m_response.clear();
    m_response.append((const char *)data, size);
    m_code = code;
  }

  void process() {
    HttpRequestHandler handler(0);
    for (unsigned int i = 0; i < 100; i++) {
      handler.run(this);
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

  if (getaddrinfo(nullptr, folly::to<std::string>(s_server_port).c_str(),
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
  WITH_PREBOUND_SOCKET(VSR("<?hh print 'Hello, World!';",
      "Hello, World!"));
  return true;
}

bool TestServer::TestTakeoverServer() {
  // start a server
  snprintf(s_filename, MAXPATHLEN, "/tmp/hphp_takeover_XXXXXX");
  auto const tmpfd = mkstemp(s_filename);
  close(tmpfd);

  s_func.reset(new AsyncFunc<TestServer>(this, &TestServer::RunServer));
  s_func->start();

  // Wait for the server to actually start
  HttpClient http;
  StringBuffer response;
  req::vector<String> responseHeaders;
  auto url = "http://127.0.0.1:" + folly::to<std::string>(s_server_port) +
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
  VSR("<?hh print 'Hello, World!';",
      "Hello, World!");
  unlink(s_filename);
  s_filename[0] = 0;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

struct EchoHandler final : RequestHandler {
  explicit EchoHandler(int timeout) : RequestHandler(timeout) {}
  // implementing RequestHandler
  void handleRequest(Transport *transport) override {
    g_context.getCheck();
    const HeaderMap& headers = transport->getHeaders();

    std::string response;
    response = "\nGET param: name = ";
    response += transport->getParam("name");

    if (transport->getMethod() == Transport::Method::POST) {
      size_t size = 0;
      auto const data = (const char *)transport->getPostData(size);
      response += "\nPOST data: ";
      response += std::string(data, size);
    }

    for (HeaderMap::const_iterator iter = headers.begin();
         iter != headers.end(); ++iter) {
      response += "\nHeader: ";
      response += iter->first;
      for (unsigned int i = 0; i < iter->second.size(); i++) {
        response += "\n";
        response += folly::to<std::string>(i);
        response += ": ";
        response += iter->second[i];
      }
    }

    transport->addHeader("Custom", "blah");
    transport->sendString(response);
    hphp_memory_cleanup();
  }
  void abortRequest(Transport *transport) override {
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
  auto url = "http://127.0.0.1:" + folly::to<std::string>(s_server_port) +
    "/echo?name=value";

  static const StaticString s_Custom_colon_blah("Custom: blah");

  for (int i = 0; i < 10; i++) {
    HttpClient http;
    StringBuffer response;
    req::vector<String> responseHeaders;
    int code = http.get(url.c_str(), response, &headers, &responseHeaders);
    VS(code, 200);
    VS(response.data(),
       ("\nGET param: name = value"
        "\nHeader: Cookie"
        "\n0: c1=v1;c2=v2;"
        "\n1: c3=v3;c4=v4;"
        "\nHeader: Accept"
        "\n0: */*"
        "\nHeader: Host"
        "\n0: 127.0.0.1:" + folly::to<std::string>(s_server_port)).c_str());

    bool found = false;
    for (unsigned int i2 = 0; i2 < responseHeaders.size(); i2++) {
      if (responseHeaders[i2] == s_Custom_colon_blah) {
        found = true;
      }
    }
    VERIFY(found);
  }
  for (int i = 0; i < 10; i++) {
    HttpClient http;
    StringBuffer response;
    req::vector<String> responseHeaders;
    int code = http.post(url.c_str(), "postdata", 8, response, &headers,
                         &responseHeaders);
    VS(code, 200);
    VS(response.data(),
       ("\nGET param: name = value"
        "\nPOST data: postdata"
        "\nHeader: Content-Type"
        "\n0: application/x-www-form-urlencoded"
        "\nHeader: Cookie"
        "\n0: c1=v1;c2=v2;"
        "\n1: c3=v3;c4=v4;"
        "\nHeader: Accept"
        "\n0: */*"
        "\nHeader: Content-Length"
        "\n0: 8"
        "\nHeader: Host"
        "\n0: 127.0.0.1:" + folly::to<std::string>(s_server_port)).c_str());

    bool found = false;
    for (unsigned int i2 = 0; i2 < responseHeaders.size(); i2++) {
      if (responseHeaders[i2] == s_Custom_colon_blah) {
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
  VSGETP("<?hh\n"
         "function f() { return 100; }\n",
         "100",
         "f?auth=test",
         s_rpc_port);

  // array output
  VSGETP("<?hh\n"
         "function f($a) { return array(1, 2, 3, $a); }\n",
         "[1,2,3,\"hello\"]",
         "f?auth=test&p=\"hello\"",
         s_rpc_port);

  // associate arrays
  VSGETP("<?hh\n"
         "function f($a, $b) { return array_merge($a, $b); }\n",
         "{\"a\":1,\"0\":2,\"1\":1,\"2\":2}",
         "f?auth=test&p={\"a\":1,\"1\":2}&p=[1,2]",
         s_rpc_port);

  // builtin function and static method
  VSGETP("<?hh\n"
         "class A { static function f($a) { return $a; } }\n",
         "100",
         "call_user_func?auth=test&p=\"A::f\"&p=100",
         s_rpc_port);

  // invoking a file, with NO json encoding
  // "int(100)" is printed twice, one from warmup, and the other from include
  VSGETP("<?hh\n"
         "var_dump(100);\n",
         "int(100)\n"
         "int(100)\n",
         "?include=string&output=1&auth=test",
         s_rpc_port);

  VSGETP("<?hh\n"
         "var_dump(isset($_ENV['HPHP_RPC']));\n",
         "bool(true)\n"
         "bool(true)\n",
         "?include=string&output=1&auth=test",
         s_rpc_port);

  return true;
}

bool TestServer::TestXboxServer() {
  VSGET("<?hh\n"
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
  VSGET("<?hh\n"
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
  VSGET("<?hh\n"
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

  VSGET("<?hh\n"
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
