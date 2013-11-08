/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "thrift/lib/cpp/transport/TSocketPool.h"
#include "thrift/lib/cpp/transport/TTransportException.h"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace apache { namespace thrift { namespace transport {

using namespace std;

using std::shared_ptr;

/**
 * TSocketPoolServer implementation
 *
 */
TSocketPoolServer::TSocketPoolServer()
  : host_(""),
    port_(0),
    socket_(-1),
    lastFailTime_(0),
    consecutiveFailures_(0) {}

/**
 * Constructor for TSocketPool server
 */
TSocketPoolServer::TSocketPoolServer(const string &host, int port)
  : host_(host),
    port_(port),
    socket_(-1),
    lastFailTime_(0),
    consecutiveFailures_(0) {}

/**
 * TSocketPool implementation.
 *
 */

TSocketPool::TSocketPool() : TSocket(),
  numRetries_(1),
  retryInterval_(60),
  maxConsecutiveFailures_(1),
  randomize_(true),
  alwaysTryLast_(true),
  maxServersToTry_(UINT_MAX) {
}

TSocketPool::TSocketPool(const vector<string> &hosts,
                         const vector<int> &ports) : TSocket(),
  numRetries_(1),
  retryInterval_(60),
  maxConsecutiveFailures_(1),
  randomize_(true),
  alwaysTryLast_(true),
  maxServersToTry_(UINT_MAX)
{
  if (hosts.size() != ports.size()) {
    GlobalOutput("TSocketPool::TSocketPool: hosts.size != ports.size");
    throw TTransportException(TTransportException::BAD_ARGS);
  }

  for (unsigned int i = 0; i < hosts.size(); ++i) {
    addServer(hosts[i], ports[i]);
  }
}

TSocketPool::TSocketPool(const vector<pair<string, int> >& servers) : TSocket(),
  numRetries_(1),
  retryInterval_(60),
  maxConsecutiveFailures_(1),
  randomize_(true),
  alwaysTryLast_(true),
  maxServersToTry_(UINT_MAX)
{
  for (unsigned i = 0; i < servers.size(); ++i) {
    addServer(servers[i].first, servers[i].second);
  }
}

TSocketPool::TSocketPool(const vector< shared_ptr<TSocketPoolServer> >& servers) : TSocket(),
  servers_(servers),
  numRetries_(1),
  retryInterval_(60),
  maxConsecutiveFailures_(1),
  randomize_(true),
  alwaysTryLast_(true),
  maxServersToTry_(UINT_MAX)
{
}

TSocketPool::TSocketPool(const string& host, int port) : TSocket(),
  numRetries_(1),
  retryInterval_(60),
  maxConsecutiveFailures_(1),
  randomize_(true),
  alwaysTryLast_(true),
  maxServersToTry_(UINT_MAX)
{
  addServer(host, port);
}

TSocketPool::~TSocketPool() {
  vector< shared_ptr<TSocketPoolServer> >::const_iterator iter = servers_.begin();
  vector< shared_ptr<TSocketPoolServer> >::const_iterator iterEnd = servers_.end();
  for (; iter != iterEnd; ++iter) {
    setCurrentServer(*iter);
    TSocketPool::close();
  }
}

void TSocketPool::addServer(const string& host, int port) {
  servers_.push_back(shared_ptr<TSocketPoolServer>(new TSocketPoolServer(host, port)));
}

void TSocketPool::addServer(shared_ptr<TSocketPoolServer> &server) {
  if (server) {
    servers_.push_back(server);
  }
}

void TSocketPool::setServers(const vector< shared_ptr<TSocketPoolServer> >& servers) {
  servers_ = servers;
}

void TSocketPool::getServers(vector< shared_ptr<TSocketPoolServer> >& servers) {
  servers = servers_;
}

int TSocketPool::getCurrentServerPort() {
  return currentServer_->port_;
}

std::string TSocketPool::getCurrentServerHost() {
  return currentServer_->host_;
}

void TSocketPool::setNumRetries(int numRetries) {
  numRetries_ = numRetries;
}

void TSocketPool::setRetryInterval(int retryInterval) {
  retryInterval_ = retryInterval;
}


void TSocketPool::setMaxConsecutiveFailures(int maxConsecutiveFailures) {
  maxConsecutiveFailures_ = maxConsecutiveFailures;
}

void TSocketPool::setRandomize(bool randomize) {
  randomize_ = randomize;
}

void TSocketPool::setAlwaysTryLast(bool alwaysTryLast) {
  alwaysTryLast_ = alwaysTryLast;
}

void TSocketPool::setCurrentServer(const shared_ptr<TSocketPoolServer> &server) {
  currentServer_ = server;
  host_ = server->host_;
  port_ = server->port_;
  socket_ = server->socket_;
}

void TSocketPool::setMaxServersToTry(unsigned int maxServersToTry) {
  maxServersToTry_ = maxServersToTry;
}


/**
 * This function throws an exception if socket open fails. When socket
 * opens fails, the socket in the current server is reset.
 * The number of servers to try is limited by maxServersToTry.
 *
 * @author Jason Sobel <jsobel@facebook.com>
 * @author Guizhen Yang <gyang@facebook.com>
 */
// TODO: without apc we ignore a lot of functionality from the php version
void TSocketPool::open() {
  unsigned int numServers = servers_.size();

  if (numServers == 0) {
    socket_ = -1;
    throw TTransportException(TTransportException::NOT_OPEN,
                              "no servers in socket pool");
  }

  if (isOpen()) {
    return;
  }

  if (randomize_ && numServers > 1) {
    random_shuffle(servers_.begin(), servers_.end());
  }

  unsigned int numServersToTry = min(maxServersToTry_,
                                 numServers);

  for (int i = 0; i < numServersToTry; ++i) {
    shared_ptr<TSocketPoolServer>& server = servers_[i];
    // Impersonate the server socket
    setCurrentServer(server);

    if (isOpen()) {
      // already open means we're done
      return;
    }

    bool retryIntervalPassed = (server->lastFailTime_ == 0);
    bool isLastServer = alwaysTryLast_ ? (i == numServersToTry - 1) : false;

    if (server->lastFailTime_ > 0) {
      // The server was marked as down, so check if enough time has elapsed to retry
      int elapsedTime = time(nullptr) - server->lastFailTime_;
      if (elapsedTime > retryInterval_) {
        retryIntervalPassed = true;
      }
    }

    if (retryIntervalPassed || isLastServer) {
      for (int j = 0; j < numRetries_; ++j) {
        try {
          TSocket::open();
        } catch (const TException& e) {
          string errStr =
            "TSocketPool::open failed " + getSocketInfo() + ": " + e.what();
          GlobalOutput(errStr.c_str());
          socket_ = -1;
          continue;
        } catch (...) {
          GlobalOutput("TSocketPool::open failed due to unknown exception");
          socket_ = -1;
          continue;
        }

        // Copy over the opened socket so that we can keep it persistent
        server->socket_ = socket_;
        // reset lastFailTime_ is required
        server->lastFailTime_ = 0;
        // success
        return;
      }

      ++server->consecutiveFailures_;
      if (server->consecutiveFailures_ > maxConsecutiveFailures_) {
        // Mark server as down
        server->consecutiveFailures_ = 0;
        server->lastFailTime_ = time(nullptr);
      }
    }
  }

  GlobalOutput("TSocketPool::open: all connections failed");

  std::ostringstream os;
  os << "all connections failed (tried " << numServersToTry << " servers)";
  throw TTransportException(TTransportException::NOT_OPEN, os.str());
}

/**
 * @author Guizhen Yang <gyang@facebook.com>
 */
void TSocketPool::close() {
  TSocket::close();
  if (currentServer_) {
    currentServer_->socket_ = -1;
  }
}

}}} // apache::thrift::transport
