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
#include "thrift/lib/cpp/async/TAsyncSignalHandler.h"

#include "thrift/lib/cpp/Thrift.h"
#include "thrift/lib/cpp/async/TEventBase.h"

#include <boost/lexical_cast.hpp>

using std::make_pair;
using std::pair;
using std::string;

namespace apache { namespace thrift { namespace async {

TAsyncSignalHandler::TAsyncSignalHandler(TEventBase* eventBase)
  : eventBase_(eventBase) {
}

TAsyncSignalHandler::~TAsyncSignalHandler() {
  // Unregister any outstanding events
  for (SignalEventMap::iterator it = signalEvents_.begin();
       it != signalEvents_.end();
       ++it) {
    event_del(&it->second);
  }
}

void TAsyncSignalHandler::registerSignalHandler(int signum) {
  pair<SignalEventMap::iterator, bool> ret =
    signalEvents_.insert(make_pair(signum, event()));
  if (!ret.second) {
    // This signal has already been registered
    throw TLibraryException("handler already registered for signal " +
                            boost::lexical_cast<string>(signum));
  }

  struct event* ev = &(ret.first->second);
  try {
    signal_set(ev, signum, libeventCallback, this);
    if (event_base_set(eventBase_->getLibeventBase(), ev) != 0 ) {
      throw TLibraryException("error initializing event handler for signal " +
                              boost::lexical_cast<string>(signum));
    }

    if (event_add(ev, nullptr) != 0) {
      throw TLibraryException("error adding event handler for signal " +
                              boost::lexical_cast<string>(signum));
    }
  } catch (...) {
    signalEvents_.erase(ret.first);
    throw;
  }
}

void TAsyncSignalHandler::unregisterSignalHandler(int signum) {
  SignalEventMap::iterator it = signalEvents_.find(signum);
  if (it == signalEvents_.end()) {
    throw TLibraryException("unable to unregister handler for signal " +
                            boost::lexical_cast<string>(signum) +
                            ": signal not registered");
  }

  event_del(&it->second);
  signalEvents_.erase(it);
}

void TAsyncSignalHandler::libeventCallback(int signum, short events,
                                           void* arg) {
  TAsyncSignalHandler* handler = static_cast<TAsyncSignalHandler*>(arg);
  handler->signalReceived(signum);
}

}}} // apache::thrift::async
