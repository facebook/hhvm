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

#include "hphp/test/ext/test_fastcgi.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/util/util.h"
#include "hphp/util/process.h"
#include "hphp/compiler/option.h"
#include "hphp/util/async-func.h"
#include "hphp/runtime/ext/ext_curl.h"
#include "hphp/runtime/ext/ext_network.h"
#include "hphp/runtime/ext/ext_socket.h"
#include "hphp/runtime/ext/ext_options.h"
#include "hphp/runtime/ext/ext_json.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/server/fastcgi/fastcgi-server.h"
#include <tuple>
#include <iostream>
#include <memory>
#include <cmath>
#include <cstdio>

using namespace HPHP;

#define PORT_MIN 7300
#define PORT_MAX 7400

///////////////////////////////////////////////////////////////////////////////

#define ADD_FILE(path) \
  if (!Count(AddFile(path, __FILE__,__LINE__))) { \
    return false; \
  }

#define LOAD_JSON(mx, s) \
  if (!Count(LoadExchangeFromJson( \
               std::string("test/ext/fastcgi/") + s, \
               mx, __FILE__,__LINE__))) { \
    return false; \
  }

#define EXCHANGE(messages, reps) \
  if (!Count(VerifyExchange(messages, reps, __FILE__,__LINE__))) { \
    return false; \
  }

///////////////////////////////////////////////////////////////////////////////

bool TestMessage::bodyFromStr(const String& input) {
  if (input.size() % 2 != 0) {
    printf("Invalid message of odd length: %s\n", input.c_str());
    return false;
  }
  m_body.clear();
  m_mask.clear();
  for (int i = 0; i < input.size(); i += 2) {
    if (input[i] == '?' && input[i + 1] == '?') {
      m_body.push_back(0x00);
      m_mask.push_back(0x00);
    } else {
      int x;
      if (sscanf(input.c_str() + i, "%2x", &x) != 1) {
        printf("Invalid message chunk: %.*s\n",
               2, input.c_str() + i);
        return false;
      }
      m_body.push_back(x);
      m_mask.push_back(0xff);
    }
  }
  return true;
}

bool TestMessage::fromJson(CVarRef json) {
  if (!json.isArray()) {
    printf("Invalid format of a message\n");
    return false;
  }
  Array arr = json.toArray();
  if (!arr.exists(String("command"))) {
    printf("Key 'command' missing from the message\n");
    return false;
  }
  if (!arr[String("command")].isString()) {
    printf("Invalid command in message\n");
    return false;
  }
  String command = arr[String("command")].toString();
  if (command == String("send") || command == String("recv")) {
    if (command == String("send")) {
      m_command = Command::SEND;
    } else {
      CHECK(command == String("recv"));
      m_command = Command::RECV;
    }
    if (arr.exists(String("message"))) {
      if (!arr[String("message")].isString()) {
        printf("Invalid value for key 'message'\n");
        return false;
      }
      String message = arr[String("message")].toString();
      bodyFromStr(message);
    } else if (arr.exists(String("length"))) {
      if (!arr[String("length")].isInteger()) {
        printf("Invalid value for key 'length'\n");
        return false;
      }
      int length = arr[String("length")].toInt32();
      m_body.resize(length);
      m_mask.clear();
      m_mask.resize(length);
    } else {
      printf("Keys 'message' or 'length' missing from a message\n");
      return false;
    }
  } else if (command == String("close")) {
    m_command = Command::CLOSE;
  } else if (command == String("recv_close")) {
    m_command = Command::RECV_CLOSE;
  } else if (command == String("call")) {
    m_command = Command::CALL;
    if (!arr.exists(String("args"))) {
      printf("Key 'args' missing from a CALL message\n");
      return false;
    }
    if (!arr[String("args")].isArray()) {
      printf("Invalid key 'args' in a CALL message\n");
      return false;
    }
    m_args.clear();
    Array args = arr[String("args")].toArray();
    for (ArrayIter it(args); it; ++it) {
      if (!it.first().isString()) {
        printf("Invalid element of 'args'\n");
        return false;
      }
      m_args[it.first().toString().data()] = it.second();
    }
  } else {
    printf("Invalid commmand: %s\n", command.data());
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestMessageExchange::fromJson(CVarRef json) {
  if (json.isNull()) {
    printf("Not a valid JSON\n");
    return false;
  }
  if (!json.isArray()) {
    printf("Invalid format of the message exchange\n");
    return false;
  }
  for (ArrayIter it(json.toArray()); it; ++it) {
    if (!it.first().isInteger()) {
      printf("Invalid format of the message exchange\n");
      return false;
    }
    int message_key = it.first().toInt32();
    TestMessage message;
    if (!message.fromJson(it.second())) {
      printf("Invalid message #%d\n", message_key);
      return false;
    }
    m_messages.push_back(message);
  }
  return true;
}

TestMessageExchange TestMessageExchange::aggregate() const {
  TestMessageExchange result;

  for (auto it = m_messages.begin();
       it != m_messages.end();) {
    if (it->m_command != TestMessage::Command::SEND) {
      result.m_messages.push_back(*it);
      ++it;
      continue;
    }
    TestMessage message;
    message.m_command = it->m_command;
    auto it2 = it;
    for (; it2 != m_messages.end(); ++it2) {
      if (it2->m_command == it->m_command) {
        std::copy(it2->m_body.begin(),
                  it2->m_body.end(),
                  std::back_inserter(message.m_body));
        std::copy(it2->m_mask.begin(),
                  it2->m_mask.end(),
                  std::back_inserter(message.m_mask));
      } else {
        break;
      }
    }
    result.m_messages.push_back(message);
    it = it2;
  }

  return result;
}

TestMessageExchange TestMessageExchange::split() const {
  TestMessageExchange result;
  for (auto it = m_messages.begin();
       it != m_messages.end();
       ++it) {
    int total_len = it->m_body.size();
    CHECK(it->m_body.size() == it->m_mask.size());

    if (it->m_command == TestMessage::Command::SEND) {
      int i = 0;
      int split_len = rand() % total_len + 1;
      if (split_len < total_len / 1000) {
        split_len = total_len / 1000;
      }
      while (i < total_len) {
        int len = rand() % std::min(split_len, (total_len - i)) + 1;
        TestMessage message;
        message.m_command = it->m_command;
        for (int j = i; j < i + len; ++j) {
          message.m_body.push_back(it->m_body[j]);
          message.m_mask.push_back(it->m_mask[j]);
        }
        result.m_messages.push_back(message);
        i += len;
      }
    } else {
      result.m_messages.push_back(*it);
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////

TestFastCGIServer::TestFastCGIServer() {}

bool TestFastCGIServer::LoadExchangeFromJson(const std::string& path,
                                             TestMessageExchange& result,
                                             const char* file,
                                             int line) {
  std::ifstream f(path.c_str());
  if (!f) {
    printf("Unable to open %s for reading. Run this test from hphp/.\n",
           path.c_str());
    return false;
  }
  std::string json_str((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
  Variant json = f_json_decode(json_str, true);
  if (!result.fromJson(json)) {
    printf("Error loading message exchange file: %s\n", path.c_str());
    return false;
  }
  return true;
}

bool TestFastCGIServer::AddFile(const std::string& path,
                                const char* file,
                                int line) {
  String source("test/ext/fastcgi/" + path);
  String dest("runtime/tmp/" + path);
  if (!f_copy(source, dest)) {
    printf("Unable to copy file from source: %s "
           "to destination: %s. Run this test from hphp/.\n",
           source.data(),
           dest.data());
    return false;
  }
  return true;
}

bool TestFastCGIServer::VerifyExchange(const TestMessageExchange& mx,
                                       int reps,
                                       const char* file,
                                       int line) {

  Variant cwd = f_getcwd();
  CHECK(cwd.isString());

  CHECK(!mx.m_messages.empty());
  if (!CleanUp()) {
    return false;
  }

  bool result = true;

  for (int i = 0; i < reps; ++i) {

    AsyncFunc<TestFastCGIServer> func(this, &TestFastCGIServer::RunServer);
    func.start();

    WaitForOpenPort("127.0.0.1", m_adminPort);
    WaitForOpenPort("127.0.0.1", m_serverPort);

    Variant tcp_proto = f_getprotobyname(String("tcp"));

    TestMessageExchange mx2 = mx;
    if (i > 0) {
      mx2 = mx2.aggregate().split();
    }

    Variant socket;
    if (!tcp_proto.isInteger()) {
      printf("Invalid return value from getprotobyname");
      result = false;
    } else {
      socket = f_socket_create(AF_INET,
                               SOCK_STREAM,
                               tcp_proto.toInt32());
    }
    if (!socket.isResource()) {
      printf("Unable to initialize a socket\n");
      result = false;
    } else if (!f_socket_connect(socket.asCResRef(),
                                 "localhost",
                                 m_serverPort)) {
      printf("Unable to connect to server.\n");
      result = false;
    }

    for (auto it = mx2.m_messages.begin();
         result && it != mx2.m_messages.end();
         ++it) {
      int len = it->m_body.size();
      CHECK(it->m_body.size() == it->m_mask.size());

      switch (it->m_command) {
        case TestMessage::Command::SEND: {
          printf("Sending a message, length=%d\n", len);
          String body_buffer = String(
              reinterpret_cast<const char*>(it->m_body.data()),
              len,
              CopyString);
          Variant sent_len = f_socket_send(socket.asCResRef(),
                                           body_buffer,
                                           len,
                                           0);
          int usecs = rand() % 50000;
          usleep(usecs); // Waiting on Nade's algorithm
          if (!sent_len.isInteger()) {
            printf("Error while sending payload\n");
            result = false;
          } else if (sent_len.toInt32() < len) {
            printf("Data truncated while sending\n");
            printf("Sent length: %d, expected: %d\n",
                   sent_len.toInt32(),
                   (int) len);
            result = false;
          } else {
            printf("Successfully sent a message\n");
            CHECK(sent_len.toInt32() == len);
          }
          break;
        }

        case TestMessage::Command::RECV: {
          printf("Receiving a message\n");
          VRefParamValue actual_body;
          Variant recv_len = f_socket_recv(socket.asCResRef(),
                                           actual_body,
                                           len,
                                           0);
          if (!recv_len.isInteger()) {
            printf("Error while receiving payload\n");
            result = false;
          } else if (recv_len.toInt32() < len) {
            printf("Too little data received\n");
            result = false;
          } else {
            CHECK(recv_len.toInt32() == len);
            CHECK(actual_body.isString());
            for (int i = 0; i < len; ++i) {
              if ((actual_body.toString()[i] & it->m_mask[i]) !=
                  (it->m_body[i] & it->m_mask[i])) {
                printf("Received payload mismatch at offset: %d "
                       "actual value: %02x, expected: %02x, "
                       "mask was: %02x\n",
                       i,
                       actual_body.toString()[i] & it->m_mask[i],
                       it->m_body[i] & it->m_mask[i],
                       it->m_mask[i]);
                result = false;
              }
            }
            printf("Successfully read a message\n");
          }
          break;
        }

        case TestMessage::Command::CLOSE: {
          break;
        }

        case TestMessage::Command::RECV_CLOSE: {
          VRefParamValue actual_body;
          Variant recv_len = f_socket_recv(socket.asResRef(),
                                           actual_body, 1, 0);
          if (recv_len.isInteger()) {
            printf("Connection not closed by the remote side\n");
            result = false;
          }
          break;
        }

        default:
          printf("Invalid message command\n");
          result = false;
      }
    }

    if (socket.isResource()) {
      f_socket_close(socket.asCResRef());
      socket = false;
    }

    AsyncFunc<TestFastCGIServer>(this, &TestFastCGIServer::StopServer).run();
    func.waitForEnd();
  }

  return result;
}

void TestFastCGIServer::RunServer() {
  string out, err;
  string portConfig = "-vServer.Port=" + lexical_cast<string>(m_serverPort);
  string adminConfig = "-vAdminServer.Port=" +
    lexical_cast<string>(m_adminPort);
  string rpcConfig = "-vSatellites.rpc.Port=" +
    lexical_cast<string>(m_rpcPort);
  string fd = lexical_cast<string>(m_inheritFd);

  const char *argv[] = {
    "", "--mode=server", "--config=test/ext/config-server.hdf",
    "-vServer.Type=fastcgi",
    portConfig.c_str(), adminConfig.c_str(), rpcConfig.c_str(),
    nullptr
  };

  if (Option::EnableEval < Option::FullEval) {
    argv[0] = "runtime/tmp/TestFastCGIServer/test";
  } else {
    argv[0] = HHVM_PATH;
  }

  Process::Exec(argv[0], argv, nullptr, out, &err);

  printf("Output: -----------------------\n%s\n", out.c_str());
  printf("Error: ------------------------\n%s\n", err.c_str());
  printf("-------------------------------\n");
}

void TestFastCGIServer::StopServer() {
  TestMessageExchange mx;
  CHECK(LoadExchangeFromJson(std::string("test/ext/fastcgi/admin_stop.json"),
                             mx, __FILE__,__LINE__));

  Variant socket;
  Variant admin_socket;
  for (int i = 0; i < 10; i++) {
    Variant tcp_proto = f_getprotobyname(String("tcp"));

    if (socket.isResource()) {
      f_socket_close(socket.asCResRef());
      socket = false;
    }
    if (admin_socket.isResource()) {
      f_socket_close(admin_socket.asCResRef());
      admin_socket = false;
    }

    if (!tcp_proto.isInteger()) {
      printf("Invalid return value from getprotobyname");
      continue;
    } else {
      socket = f_socket_create(AF_INET,
                               SOCK_STREAM,
                               tcp_proto.toInt32());
    }
    if (!socket.isResource()) {
      printf("Unable to initialize a socket\n");
      continue;
    } else if (!f_socket_connect(socket.asCResRef(),
                                 "localhost",
                                 m_adminPort)) {
      printf("Unable to connect to admin server.\n");
      continue;
    }

    for (auto it = mx.m_messages.begin();
         it != mx.m_messages.end();
         ++it) {

      CHECK(it->m_command == TestMessage::Command::SEND);

      int len = it->m_body.size();
      String body_buffer = String(
        reinterpret_cast<const char*>(it->m_body.data()),
        len,
        CopyString);
      Variant sent_len = f_socket_send(socket.asCResRef(),
                                       body_buffer,
                                       body_buffer.size(),
                                       0);

      if (!sent_len.isInteger()) {
          printf("Error while sending payload\n");
        continue;
      } else if (sent_len.toInt32() < len) {
        printf("Data truncated while sending\n");
        printf("Sent length: %d, expected: %d\n",
               sent_len.toInt32(),
               (int) len);
        continue;
      }
      printf("Successfully sent a message\n");
    }

    WaitForClosedPort("127.0.0.1", m_adminPort);
    WaitForClosedPort("127.0.0.1", m_serverPort);
    usleep(500000); // To be sure.

    admin_socket = f_socket_create(AF_INET,
                                   SOCK_STREAM,
                                   tcp_proto.toInt32());

    if (!admin_socket.isResource()) {
      printf("Unable to initialize a socket\n");
      continue;
    }
    if (!f_socket_connect(admin_socket.asCResRef(),
                          "localhost",
                          m_serverPort)) {
      break;
    }
  }

  if (socket.isResource()) {
    f_socket_close(socket.asCResRef());
  }
  if (admin_socket.isResource()) {
    f_socket_close(admin_socket.asCResRef());
  }
}

void TestFastCGIServer::WaitForClosedPort(const char* host,
                                          int port) {
  for (int i = 0; i < 100; ++i) {
    Variant result = f_fsockopen(host, port);
    if (!result.isResource()) {
      return;
    }
    f_socket_close(result.asCResRef());
    usleep(100000); // 100 ms
  }
  throw std::runtime_error("Waiting for closed port failed!");
}

void TestFastCGIServer::WaitForOpenPort(const char* host,
                                        int port) {
  for (int i = 0; i < 100; ++i) {
    Variant result = f_fsockopen(host, port);
    if (result.isResource()) {
      f_socket_close(result.asCResRef());
      return;
    }
    usleep(100000); // 100 ms
  }
  throw std::runtime_error("Waiting for open port failed!");
}

int TestFastCGIServer::FindFreePort(const char* host,
                                    int port_min, int port_max) {

  for (int port = port_min; ; port++) {
    Variant result = f_fsockopen(host, port);
    if (!result.isResource()) {
      return port;
    };
  }
  throw std::runtime_error("No free ports found");
}

void TestFastCGIServer::SetupServerPorts() {
  m_serverPort = FindFreePort("127.0.0.1", PORT_MIN, PORT_MAX);
  m_adminPort = FindFreePort("127.0.0.1", m_serverPort + 1, PORT_MAX);
  m_rpcPort = FindFreePort("127.0.0.1", m_adminPort + 1, PORT_MAX);
}

bool TestFastCGIServer::RunTests(const std::string &which) {
  bool ret = true;

  SetupServerPorts();
  RUN_TEST(TestNginxHelloWorld);
  RUN_TEST(TestNginxFileUpload);
  RUN_TEST(TestNginxNotFound);
  RUN_TEST(TestApacheHelloWorld);
  RUN_TEST(TestApacheNotFound);

  return ret;
}

bool TestFastCGIServer::TestNginxHelloWorld() {
  TestMessageExchange mx;
  ADD_FILE("hello_world.php");
  LOAD_JSON(mx, "nginx_hello_world.json");
  EXCHANGE(mx, 10);
  return true;
}

bool TestFastCGIServer::TestNginxFileUpload() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "nginx_file_upload.json");
  EXCHANGE(mx, 2);
  return true;
}

bool TestFastCGIServer::TestNginxNotFound() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "nginx_not_found.json");
  EXCHANGE(mx, 10);
  return true;
}

bool TestFastCGIServer::TestApacheHelloWorld() {
  TestMessageExchange mx;
  ADD_FILE("hello_world.php");
  LOAD_JSON(mx, "apache_hello_world.json");
  EXCHANGE(mx, 10);
  return true;
}

bool TestFastCGIServer::TestApacheNotFound() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "apache_not_found.json");
  EXCHANGE(mx, 10);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

