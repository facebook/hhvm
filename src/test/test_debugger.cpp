/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <test/test_debugger.h>
#include <test/test_server.h>
#include <runtime/ext/ext_curl.h>
#include <runtime/ext/ext_preg.h>
#include <runtime/ext/ext_options.h>
#include <runtime/base/zend/zend_math.h>
#include <util/async_func.h>
#include <util/process.h>

///////////////////////////////////////////////////////////////////////////////

static int get_random_port() {
  const int BasePort = 20000;
  const int PortRange = 3000;
  return BasePort + rand() % PortRange;
}

TestDebugger::TestDebugger() {
  srand(math_generate_seed());
  m_serverPort = get_random_port();
  do {
    m_adminPort = get_random_port();
  } while (m_adminPort == m_serverPort);
  do {
    m_debugPort = get_random_port();
  } while (m_debugPort == m_serverPort ||
           m_debugPort == m_adminPort);
}

///////////////////////////////////////////////////////////////////////////////

bool TestDebugger::RunTests(const std::string &which) {
  bool ret = true;

  unlink("/tmp/hphpd_test_error.log");
  AsyncFunc<TestDebugger> func(this, &TestDebugger::runServer);
  func.start();

  {
    // To make sure TestSanity always get run
    std::string which = "TestSanity";
    RUN_TEST(TestSanity);
  }
  RUN_TEST(TestBasic);
  RUN_TEST(TestBreak);
  RUN_TEST(TestFlow);
  RUN_TEST(TestStack);
  RUN_TEST(TestEval);
  RUN_TEST(TestException);
  RUN_TEST(TestInfo);
  RUN_TEST(TestWebRequest);

  stopServer();
  func.waitForEnd();

  return ret;
}

bool TestDebugger::getResponse(const string& path, string& result,
                               int port /* = -1 */,
                               const string& host /* = "" */) {
  String server = "http://";
  if (host.empty()) {
    server += f_php_uname("n");
  } else {
    server += host;
  }
  server += ":" + boost::lexical_cast<string>(port > 0 ? port : m_serverPort);
  server += "/" + path;
  printf("\n  Getting URL '%s'...\n", server.get()->data());
  Variant c = f_curl_init();
  f_curl_setopt(c, k_CURLOPT_URL, server);
  f_curl_setopt(c, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_setopt(c, CURLOPT_TIMEOUT, 120);
  Variant res = f_curl_exec(c);
  if (same(res, false)) {
    printf("  Request failed\n");
    return false;
  }
  result = res.toString();
  printf("  Request succeeded, returning '%s'\n", result.c_str());
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestDebugger::TestSanity() {
  // first test, server might not be ready yet
  bool ret = false;
  for (int i = 0; i < 20; i++) {
    string result;
    if (!getResponse("hello.php", result)) {
      sleep(2);
      continue;
    }
    if (result == "Hello, World!") {
      ret = true;
    }
    break;
  }
  return Count(ret);
}

#define RUN_FILE(php) {                   \
  string result;                          \
  bool ret = false;                       \
  if (getResponse(php, result)) {         \
    ret = result == "pass";               \
  } else {                                \
    printf("failed to get response\n");   \
  }                                       \
  return Count(ret);                      \
}                                         \

bool TestDebugger::TestBasic() {
  RUN_FILE("basic.php");
}

bool TestDebugger::TestBreak() {
  RUN_FILE("break.php");
}

bool TestDebugger::TestFlow() {
  RUN_FILE("flow.php");
}

bool TestDebugger::TestStack() {
  RUN_FILE("stack.php");
}

bool TestDebugger::TestEval() {
  RUN_FILE("eval.php");
}

bool TestDebugger::TestException() {
  RUN_FILE("exception.php");
}

bool TestDebugger::TestInfo() {
  RUN_FILE("info.php");
}

static std::string getSandboxHostFormat() {
  // Assume dev servers host name are in following format:
  // <host>.<cluster>.<domain>.com
  // and we need to change to the following to match sandbox format:
  // www.<sandbox>.<host>.<domain>.com
  String hostName = f_php_uname("n");
  Array fields = f_split("\\.", hostName);
  if (fields.size() != 4) {
    return "";
  }
  String host = fields.rvalAt(0).toString();
  String domain = fields.rvalAt(1).toString() + "." +
    fields.rvalAt(2).toString();
  String suffix = fields.rvalAt(3).toString();
  string sandboxHost = "hphpd.debugger_tests." + host->toCPPString() +
                       "." + domain->toCPPString() +
                       "." + suffix->toCPPString();
  return sandboxHost;
}

bool TestDebugger::recvFromTests(char& flag) {
  const int TIMEOUT_SEC = 10;
  int fifoFd = open(m_fname.c_str(), O_RDONLY | O_NONBLOCK);
  if (fifoFd < 0) {
    return false;
  }
  while (true) {
    pollfd fds[1];
    fds[0].fd = fifoFd;
    fds[0].events = POLLIN|POLLERR|POLLHUP;
    int ret = poll(fds, 1, TIMEOUT_SEC * 1000);
    if (ret == 0) {
      // timed out
      return false;
    }
    if (ret == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        close(fifoFd);
        return false;
      }
    }
    if (fds[0].events & POLLIN) {
      read(fifoFd, &flag, 1);
      break;
    }
    close(fifoFd);
    return false;
  }
  close(fifoFd);
  // slightly wait here since the server side will send out the flag _before_
  // going to block, but we should take action _after_ it is blocked.
  usleep(30000);
  return true;
}

bool TestDebugger::TestWebRequest() {
  bool ret = false;

  // If we can't get sandbox host format right, fail early
  string sandboxHost = getSandboxHostFormat();
  if (sandboxHost.empty()) {
    return CountSkip();
  }
  string result;
  if (!getResponse("hello.php", result, -1, sandboxHost) ||
      result != "Hello, World!") {
    printf("Sanity check didn't pass on sandbox host %s, skip test\n",
           sandboxHost.c_str());
    return CountSkip();
  }

  m_fname = "/tmp/hphpd_test_fifo." +
            boost::lexical_cast<string>(Process::GetProcessId());
  if (mkfifo(m_fname.c_str(), 0666)) {
    printf("Can not make fifo %s, skip test\n", m_fname.c_str());
    return CountSkip();
  }

  m_tempResult = false;

  AsyncFunc<TestDebugger> func(this, &TestDebugger::testWebRequestHelper);
  func.start();

  char flag;
  // wait for "web_request.php" to connect and wait
  if (!recvFromTests(flag) || flag != '1') {
    printf("failed to receive from test\n");
  } else if (!getResponse("web_request_t.php", result, -1, sandboxHost) ||
      result != "request done") {
    printf("failed on web_request_t.php\n");
  } else {
    ret = true;
  }

  func.waitForEnd();

  unlink(m_fname.c_str());

  // testWebRequestHelper() should flag m_tempResult to true if succeed
  ret = ret && m_tempResult;
  return Count(ret);
}

void TestDebugger::testWebRequestHelper() {
  string result;
  if (getResponse("web_request.php", result)) {
    m_tempResult = result == "pass";
  } else {
    printf("failed to get response for web_request.php\n");
  }
  if (!m_tempResult) {
    return;
  }

  m_tempResult = false;
  // test interrupt: again starting a new thread first wait on interrupts
  AsyncFunc<TestDebugger> func(this,
                               &TestDebugger::testWebRequestHelperSignal);
  func.start();

  char flag;
  // wait for "web_request_signal.php" to connect and wait
  if (!recvFromTests(flag) || flag != '2') {
    printf("failed to receive from test\n");
    return;
  }

  if (!getResponse("web_request_interrupt.php", result) ||
      result != "interrupt done") {
    printf("failed on web_request_interrupt.php\n");
    return;
  }

  // wait for "web_request_t.php" to finish
  if (!recvFromTests(flag) || flag != '3') {
    printf("failed to receive from test\n");
    return;
  }

  if (!getResponse("web_request_interrupt.php", result) ||
      result != "interrupt done") {
    printf("failed on web_request_interrupt.php\n");
    return;
  }

  func.waitForEnd();
}

void TestDebugger::testWebRequestHelperSignal() {
  string result;
  if (getResponse("web_request_signal.php", result)) {
    m_tempResult = result == "pass";
  } else {
    printf("failed to get response for web_request_signal.php\n");
  }
}

///////////////////////////////////////////////////////////////////////////////

void TestDebugger::runServer() {
  string out, err;
  string portConfig = "-vServer.Port=" +
                      boost::lexical_cast<string>(m_serverPort);
  string srcRootConfig = "-vServer.SourceRoot=" +
                         Process::GetCurrentDirectory() +
                         "/test/debugger_tests";
  string includePathConfig = "-vServer.IncludeSearchPaths.0=" +
                             Process::GetCurrentDirectory() +
                             "/test/debugger_tests";
  string adminPortConfig = "-vAdminServer.Port=" +
                           boost::lexical_cast<string>(m_adminPort);
  string debugPortConfig = "-vEval.Debugger.Port=" +
                           boost::lexical_cast<string>(m_debugPort);
  string jitConfig = "-vEval.Jit=" +
                     boost::lexical_cast<string>(RuntimeOption::EvalJit);
  string jitUseIRConfig = "-vEval.JitUseIR=" +
                          boost::lexical_cast<string>(
                              RuntimeOption::EvalJitUseIR);

  // To emulate sandbox setup, let home to be "src/test", and user name to be
  // "debugger_tests", so that it can find the sandbox_conf there
  string sandboxHomeConfig = "-vSandbox.Home=" +
                             Process::GetCurrentDirectory() +
                             "/test";
  const char *argv[] = {"hphpd_test", "--mode=server",
                        "--config=test/config-debugger-server.hdf",
                        portConfig.c_str(),
                        srcRootConfig.c_str(),
                        includePathConfig.c_str(),
                        sandboxHomeConfig.c_str(),
                        adminPortConfig.c_str(),
                        debugPortConfig.c_str(),
                        jitConfig.c_str(),
                        jitUseIRConfig.c_str(),
                        NULL};
  printf("Running server with arguments:\n");
  for (unsigned i = 1; i < array_size(argv) - 1; ++i) {
    printf("%s ", argv[i]);
  }
  printf("\n");
  Process::Exec(HHVM_PATH, argv, NULL, out, &err);
}

void TestDebugger::stopServer() {
  for (int i = 0; i < 10; i++) {
    string result;
    if (getResponse("stop", result, m_adminPort)) {
      break;
    }
    sleep(1);
  }
}
