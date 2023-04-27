/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GFlags.h>
#include <folly/ssl/SSLSession.h>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using namespace folly;
using folly::ssl::SSLSession;

DEFINE_int32(clients, 1, "Number of simulated SSL clients");
DEFINE_int32(threads, 1, "Number of threads to spread clients across");
DEFINE_int32(requests, 2, "Total number of requests per client");
DEFINE_int32(port, 9423, "Server port");
DEFINE_bool(
    sticky,
    false,
    "A given client sends all reqs to one "
    "(random) server");
DEFINE_bool(global, false, "All clients in a thread use the same SSL session");
DEFINE_bool(handshakes, false, "Force 100% handshakes");

string f_servers[10];
int f_num_servers = 0;
int tnum = 0;

class ClientRunner {
 public:
  ClientRunner() : reqs(0), hits(0), miss(0), num(tnum++) {}
  void run();

  int reqs;
  int hits;
  int miss;
  int num;
};

class SSLCacheClient : public AsyncSocket::ConnectCallback,
                       public AsyncSSLSocket::HandshakeCB {
 private:
  EventBase* eventBase_;
  int currReq_;
  int serverIdx_;
  AsyncSocket* socket_;
  AsyncSSLSocket* sslSocket_;
  std::shared_ptr<SSLSession> session_;
  std::shared_ptr<SSLSession> pSess_;
  std::shared_ptr<SSLContext> ctx_;
  ClientRunner* cr_;

 public:
  SSLCacheClient(
      EventBase* eventBase,
      std::shared_ptr<SSLSession> pSess,
      ClientRunner* cr);
  ~SSLCacheClient() override {
    if (socket_ != nullptr) {
      if (sslSocket_ != nullptr) {
        sslSocket_->destroy();
        sslSocket_ = nullptr;
      }
      socket_->destroy();
      socket_ = nullptr;
    }
  }

  void start();

  void connectSuccess() noexcept override;

  void connectErr(const AsyncSocketException& ex) noexcept override;

  void handshakeSuc(AsyncSSLSocket* sock) noexcept override;

  void handshakeErr(
      AsyncSSLSocket* sock,
      const AsyncSocketException& ex) noexcept override;
};

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage(std::string(
      "\n\n"
      "usage: sslcachetest [options] -c <clients> -t <threads> servers\n"));
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  int reqs = 0;
  int hits = 0;
  int miss = 0;
  struct timeval start;
  struct timeval end;
  struct timeval result;

  srand((unsigned int)time(nullptr));

  for (int i = 1; i < argc; i++) {
    f_servers[f_num_servers++] = argv[i];
  }
  if (f_num_servers == 0) {
    cout << "require at least one server\n";
    return 1;
  }

  gettimeofday(&start, nullptr);
  if (FLAGS_threads == 1) {
    ClientRunner r;
    r.run();
    gettimeofday(&end, nullptr);
    reqs = r.reqs;
    hits = r.hits;
    miss = r.miss;
  } else {
    std::vector<ClientRunner> clients;
    std::vector<std::thread> threads;
    for (int t = 0; t < FLAGS_threads; t++) {
      threads.emplace_back([&] { clients[t].run(); });
    }
    for (auto& thr : threads) {
      thr.join();
    }
    gettimeofday(&end, nullptr);

    for (const auto& client : clients) {
      reqs += client.reqs;
      hits += client.hits;
      miss += client.miss;
    }
  }

  timersub(&end, &start, &result);

  cout << "Requests: " << reqs << endl;
  cout << "Handshakes: " << miss << endl;
  cout << "Resumes: " << hits << endl;
  cout << "Runtime(ms): " << result.tv_sec << "." << result.tv_usec / 1000
       << endl;

  cout << "ops/sec: "
       << (reqs * 1.0) /
          ((double)result.tv_sec * 1.0 + (double)result.tv_usec / 1000000.0)
       << endl;

  return 0;
}

void ClientRunner::run() {
  EventBase eb;
  std::list<SSLCacheClient*> clients;
  std::shared_ptr<SSLSession> session = nullptr;

  for (int i = 0; i < FLAGS_clients; i++) {
    SSLCacheClient* c = new SSLCacheClient(&eb, session, this);
    c->start();
    clients.push_back(c);
  }

  eb.loop();

  for (auto it = clients.begin(); it != clients.end(); it++) {
    delete *it;
  }

  reqs += hits + miss;
}

SSLCacheClient::SSLCacheClient(
    EventBase* eb,
    std::shared_ptr<SSLSession> pSess,
    ClientRunner* cr)
    : eventBase_(eb),
      currReq_(0),
      serverIdx_(0),
      socket_(nullptr),
      sslSocket_(nullptr),
      session_(nullptr),
      pSess_(pSess),
      cr_(cr) {
  ctx_.reset(new SSLContext());
  ctx_->setOptions(SSL_OP_NO_TICKET);
}

void SSLCacheClient::start() {
  if (currReq_ >= FLAGS_requests) {
    cout << "+";
    return;
  }

  if (currReq_ == 0 || !FLAGS_sticky) {
    serverIdx_ = rand() % f_num_servers;
  }
  if (socket_ != nullptr) {
    if (sslSocket_ != nullptr) {
      sslSocket_->destroy();
      sslSocket_ = nullptr;
    }
    socket_->destroy();
    socket_ = nullptr;
  }
  socket_ = new AsyncSocket(eventBase_);
  socket_->connect(this, f_servers[serverIdx_], (uint16_t)FLAGS_port);
}

void SSLCacheClient::connectSuccess() noexcept {
  sslSocket_ = new AsyncSSLSocket(
      ctx_, eventBase_, socket_->detachNetworkSocket(), false);

  if (!FLAGS_handshakes) {
    if (session_ != nullptr)
      sslSocket_->setSSLSession(session_);
    else if (FLAGS_global && pSess_ != nullptr)
      sslSocket_->setSSLSession(pSess_);
  }
  sslSocket_->sslConn(this);
}

void SSLCacheClient::connectErr(const AsyncSocketException& ex) noexcept {
  cout << "connectError: " << ex.what() << endl;
}

void SSLCacheClient::handshakeSuc(AsyncSSLSocket*) noexcept {
  if (sslSocket_->getSSLSessionReused()) {
    cr_->hits++;
  } else {
    cr_->miss++;
    session_ = sslSocket_->getSSLSession();
    if (FLAGS_global && pSess_ != nullptr && pSess_ == nullptr) {
      pSess_ = session_;
    }
  }
  if (((cr_->hits + cr_->miss) % 100) == ((100 / FLAGS_threads) * cr_->num)) {
    cout << ".";
    cout.flush();
  }
  sslSocket_->closeNow();
  currReq_++;
  this->start();
}

void SSLCacheClient::handshakeErr(
    AsyncSSLSocket*,
    const AsyncSocketException& ex) noexcept {
  cout << "handshakeError: " << ex.what() << endl;
}
