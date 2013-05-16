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

#include "hphp/test/test_debugger.h"
#include "hphp/test/test_server.h"
#include "hphp/runtime/ext/ext_curl.h"
#include "hphp/runtime/ext/ext_preg.h"
#include "hphp/runtime/ext/ext_options.h"
#include "hphp/runtime/base/zend/zend_math.h"
#include "hphp/util/async_func.h"
#include "hphp/util/process.h"

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
  if (!func.waitForEnd(10)) {
    printf("Server did not stop within 10 seconds.\n");
    func.cancel();
    func.waitForEnd(1);
    ret = false;
  }

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
  result = (std::string) res.toString();
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

// Wait for a notification from a request that it is ready for the test
// harness to proceed to another step.
// NB: the notification comes in before the debugging logic running in the
// server has a chance to continue. Every request that follows a call to this
// function should likely beging with a call to waitForClientToGetBusy() to
// ensure the client is ready to receive more interrupts.
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
  return true;
}

// The goal of this is to test a real web request, rather than simply evaling
// snippets of code like the other tests. A single web request is issued and
// debugged by multiple other requests.
bool TestDebugger::TestWebRequest() {
  bool ret = false;

  // If we can't get sandbox host format right, fail early
  string sandboxHost = getSandboxHostFormat();
  if (sandboxHost.empty()) {
    return CountSkip();
  }

  // A quick sanity check to ensure the server is still up and accepting
  // requests.
  string result;
  if (!getResponse("hello.php", result, -1, sandboxHost) ||
      result != "Hello, World!") {
    printf("Sanity check didn't pass on sandbox host %s, skip test\n",
           sandboxHost.c_str());
    return CountSkip();
  }

  // We need to wait at various times during the test for a request to progress
  // to a certian point, at which time we can take further action. This is done
  // via this pipe.
  m_fname = "/tmp/hphpd_test_fifo." +
            boost::lexical_cast<string>(Process::GetProcessId());
  if (mkfifo(m_fname.c_str(), 0666)) {
    printf("Can not make fifo %s, skip test\n", m_fname.c_str());
    return CountSkip();
  }

  m_tempResult = false;

  // Kick off a thread to make requests for the debugger portions of this test.
  AsyncFunc<TestDebugger> func(this, &TestDebugger::testWebRequestHelperPhase1);
  func.start();

  char flag;
  // Wait for web_request_phase_1.php, the first debugger portion of the test,
  // to connect and get ready to debug.
  if (!recvFromTests(flag) || flag != '1') {
    printf("failed to receive from test\n");
  } else {
    // Now get web_request_t.php running so that web_requests.php
    // can debug it. Wait here for the entire request to finish.
    if (!getResponse("web_request_t.php", result, -1, sandboxHost) ||
        result != "request done") {
      printf("failed on web_request_t.php\n");
    } else {
      ret = true;
    }
  }

  func.waitForEnd();

  unlink(m_fname.c_str());

  // testWebRequestHelperPhase1() should flag m_tempResult to true if succeed
  ret = ret && m_tempResult;
  return Count(ret);
}

// First phase of debugging for TestWebRequest.
// This thread will issue requests to debug the the request for
// web_request_t.php. This is done with multiple requests to allow us to test
// breaking with ctrl-c.
void TestDebugger::testWebRequestHelperPhase1() {
  string result;
  // Start with the first phase of debugging. This will test request start,
  // breakpoints and inspection, and a hard breakpoint. web_request_t.php will
  // be left stopped at a hard breakpoint near the end of test_break().
  if (getResponse("web_request_phase_1.php", result)) {
    m_tempResult = result == "pass";
  } else {
    printf("failed to get response for web_request_phase_1.php\n");
  }
  if (!m_tempResult) {
    return;
  }

  m_tempResult = false;
  // Test interrupt: again starting a new thread to continue the next phase of
  // debugging logic. This will issue a request for web_request_phase_2.php.
  AsyncFunc<TestDebugger> func(this,
                               &TestDebugger::testWebRequestHelperPhase2);
  func.start();

  char flag;
  // Wait for web_request_phase_2.php to connect and get ready to debug.
  if (!recvFromTests(flag) || flag != '2') {
    printf("failed to receive from test\n");
    return;
  }

  // Issue a reqeust for web_request_interrupt.php. Web_request_phase_2.php has
  // let the original request for web_request_t.php run, and it should now be
  // spinning in an infinite loop in test_sleep(). This will issue a ctrl-c
  // which will break within that loop.
  if (!getResponse("web_request_interrupt.php", result) ||
      result != "interrupt done") {
    printf("failed on web_request_interrupt.php\n");
    return;
  }

  // Wait for web_request_phase_2.php to finish debugging the original request
  // for web_request_t.php, which will be left running after the end of PSP.
  if (!recvFromTests(flag) || flag != '3') {
    printf("failed to receive from test\n");
    return;
  }

  // Issue another request for web_request_interrupt.php. The original request
  // for web_request_t.php should be done by now, so this should interrupt in
  // the dummy sandbox. This will finally let web_request_phase_2.php complete,
  // thus completing the debugging portion of this test.
  if (!getResponse("web_request_interrupt.php", result) ||
      result != "interrupt done") {
    printf("failed on web_request_interrupt.php\n");
    return;
  }

  func.waitForEnd();
}

// Second phase of debugging for TestWebRequest.
// This thread will issue a single request for web_request_phase_2.php, which
// will complete debugging of the request for web_request_t.php.
void TestDebugger::testWebRequestHelperPhase2() {
  string result;
  if (getResponse("web_request_phase_2.php", result)) {
    m_tempResult = result == "pass";
  } else {
    printf("failed to get response for web_request_phase_2.php\n");
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

  // To emulate sandbox setup, let home to be "hphp/test", and user name to be
  // "debugger_tests", so that it can find the sandbox_conf there
  string sandboxHomeConfig = "-vSandbox.Home=" +
                             Process::GetCurrentDirectory() +
                             "/test";
  const char *argv[] = {"hphpd_test", "--mode=server",
                        "--config=test/config-debugger-server.hdf",
                        "-vEval.JitWarmupRequests=0",
                        portConfig.c_str(),
                        srcRootConfig.c_str(),
                        includePathConfig.c_str(),
                        sandboxHomeConfig.c_str(),
                        adminPortConfig.c_str(),
                        debugPortConfig.c_str(),
                        jitConfig.c_str(),
                        nullptr};
  printf("Running server with arguments:\n");
  for (unsigned i = 1; i < array_size(argv) - 1; ++i) {
    printf("%s ", argv[i]);
  }
  printf("\n");
  Process::Exec(HHVM_PATH, argv, nullptr, out, &err);
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
