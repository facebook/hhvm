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
// #define __STDC_FORMAT_MACROS

#include "thrift/lib/cpp/async/TAsyncServerSocket.h"

#include "thrift/lib/cpp/async/TEventBase.h"
#include "thrift/lib/cpp/async/TNotificationQueue.h"
#include "thrift/lib/cpp/transport/TSocketAddress.h"
#include "thrift/lib/cpp/transport/TTransportException.h"
#include "thrift/lib/cpp/util/FdUtils.h"

#include "folly/Conv.h"
#include "folly/ScopeGuard.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

using apache::thrift::transport::TSocketAddress;
using apache::thrift::transport::TTransportException;

namespace apache { namespace thrift { namespace async {

const uint32_t TAsyncServerSocket::kDefaultMaxAcceptAtOnce;
const uint32_t TAsyncServerSocket::kDefaultCallbackAcceptAtOnce;
const uint32_t TAsyncServerSocket::kDefaultMaxMessagesInQueue;

void TAsyncServerSocket::RemoteAcceptor::start(
  TEventBase* eventBase, uint32_t maxAtOnce, uint32_t maxInQueue) {
  setMaxReadAtOnce(maxAtOnce);
  queue_.setMaxQueueSize(maxInQueue);

  if (!eventBase->inRunningEventBaseThread()) {
    // This is only called when the server starts.
    // Using a bind here is fine.
    if (!eventBase->runInEventBaseThread([=](){
          callback_->acceptStarted();
          this->startConsuming(eventBase, &queue_);
        })) {
      throw TLibraryException("unable to start waiting on accept "
                              "notification queue in the specified "
                              "TEventBase thread");
    }
  } else {
    callback_->acceptStarted();
    this->startConsuming(eventBase, &queue_);
  }
}

void TAsyncServerSocket::RemoteAcceptor::stop(
  TEventBase* eventBase, AcceptCallback* callback) {
  if (eventBase) {
    if (!eventBase->runInEventBaseThread([=](){
          callback->acceptStopped();
          delete this;
        })) {
      throw TLibraryException("unable to start waiting on accept "
                              "notification queue in the specified "
                              "TEventBase thread");
    }
  } else {
    callback->acceptStopped();
    delete this;
  }
}

void TAsyncServerSocket::RemoteAcceptor::messageAvailable(
  QueueMessage&& msg) {

  switch (msg.type) {
    case MessageType::MSG_NEW_CONN:
    {
      callback_->connectionAccepted(msg.fd, msg.address);
      break;
    }
    case MessageType::MSG_ERROR:
    {
      TTransportException ex(TTransportException::INTERNAL_ERROR, msg.msg,
                             msg.err);
      callback_->acceptError(ex);
      break;
    }
    default:
    {
      T_ERROR("invalid accept notification message type %d", int(msg.type));
      TLibraryException ex("received invalid accept notification message type");
      callback_->acceptError(ex);
    }
  }
}

/*
 * TAsyncServerSocket::BackoffTimeout
 */
class TAsyncServerSocket::BackoffTimeout : public TAsyncTimeout {
 public:
  BackoffTimeout(TAsyncServerSocket* socket)
    : TAsyncTimeout(socket->getEventBase()),
      socket_(socket) {}

  virtual void timeoutExpired() noexcept {
    socket_->backoffTimeoutExpired();
  }

 private:
  TAsyncServerSocket* socket_;
};

/*
 * TAsyncServerSocket methods
 */

TAsyncServerSocket::TAsyncServerSocket(TEventBase* eventBase)
:   eventBase_(eventBase),
    accepting_(false),
    maxAcceptAtOnce_(kDefaultMaxAcceptAtOnce),
    maxNumMsgsInQueue_(kDefaultMaxMessagesInQueue),
    acceptRateAdjustSpeed_(0),
    acceptRate_(1),
    lastAccepTimestamp_(concurrency::Util::currentTime()),
    numDroppedConnections_(0),
    callbackIndex_(0),
    backoffTimeout_(nullptr),
    callbacks_(),
    keepAliveEnabled_(true),
    closeOnExec_(true),
    shutdownSocketSet_(nullptr) {
}

void TAsyncServerSocket::setShutdownSocketSet(ShutdownSocketSet* newSS) {
  if (shutdownSocketSet_ == newSS) {
    return;
  }
  if (shutdownSocketSet_) {
    for (auto& h : sockets_) {
      shutdownSocketSet_->remove(h.socket_);
    }
  }
  shutdownSocketSet_ = newSS;
  if (shutdownSocketSet_) {
    for (auto& h : sockets_) {
      shutdownSocketSet_->add(h.socket_);
    }
  }
}

TAsyncServerSocket::~TAsyncServerSocket() {
  assert(callbacks_.empty());
}

void TAsyncServerSocket::destroy() {
  for (auto& handler : sockets_) {
    T_DEBUG_L(1, "TAsyncServerSocket::destroy(this=%p, fd=%d)", this,
              handler.socket_);
  }
  assert(eventBase_ == nullptr || eventBase_->isInEventBaseThread());

  // When destroy is called, unregister and close the socket immediately
  accepting_ = false;

  for (auto& handler : sockets_) {
    handler.unregisterHandler();
    if (shutdownSocketSet_) {
      shutdownSocketSet_->close(handler.socket_);
    } else {
      ::close(handler.socket_);
    }
  }
  sockets_.clear();

  // Destroy the backoff timout.  This will cancel it if it is running.
  delete backoffTimeout_;
  backoffTimeout_ = nullptr;

  // Close all of the callback queues to notify them that they are being
  // destroyed.  No one should access the TAsyncServerSocket any more once
  // destroy() is called.  However, clear out callbacks_ before invoking the
  // accept callbacks just in case.  This will potentially help us detect the
  // bug if one of the callbacks calls addAcceptCallback() or
  // removeAcceptCallback().
  std::vector<CallbackInfo> callbacksCopy;
  callbacks_.swap(callbacksCopy);
  for (std::vector<CallbackInfo>::iterator it = callbacksCopy.begin();
       it != callbacksCopy.end();
       ++it) {
    it->consumer->stop(it->eventBase, it->callback);
  }

  // Then call TDelayedDestruction::destroy() to take care of
  // whether or not we need immediate or delayed destruction
  TDelayedDestruction::destroy();
}

void TAsyncServerSocket::attachEventBase(TEventBase *eventBase) {
  assert(eventBase_ == nullptr);
  assert(eventBase->isInEventBaseThread());

  eventBase_ = eventBase;
  for (auto& handler : sockets_) {
    handler.attachEventBase(eventBase);
  }
}

void TAsyncServerSocket::detachEventBase() {
  assert(eventBase_ != nullptr);
  assert(eventBase_->isInEventBaseThread());
  assert(!accepting_);

  eventBase_ = nullptr;
  for (auto& handler : sockets_) {
    handler.detachEventBase();
  }
}

void TAsyncServerSocket::useExistingSocket(int fd) {
  assert(eventBase_ == nullptr || eventBase_->isInEventBaseThread());

  if (sockets_.size() > 0) {
    throw TTransportException(TTransportException::ALREADY_OPEN,
                              "cannot call useExistingSocket() on a "
                              "TAsyncServerSocket that already has a socket");
  }

  // Set addressFamily_ from this socket.
  // Note that the socket may not have been bound yet, but
  // setFromLocalAddress() will still work and get the correct address family.
  // We will update addressFamily_ again anyway if bind() is called later.
  TSocketAddress address;
  address.setFromLocalAddress(fd);

  setupSocket(fd);
  sockets_.push_back(
    ServerEventHandler(eventBase_, fd, this, address.getFamily()));
  sockets_[0].changeHandlerFD(fd);
}

void TAsyncServerSocket::bind(const transport::TSocketAddress& address) {
  assert(eventBase_ == nullptr || eventBase_->isInEventBaseThread());

  // useExistingSocket() may have been called to initialize socket_ already.
  // However, in the normal case we need to create a new socket now.
  // Don't set socket_ yet, so that socket_ will remain uninitialized if an
  // error occurs.
  int fd;
  if (sockets_.size() == 0) {
    fd = createSocket(address.getFamily());
  } else if (sockets_.size() == 1) {
    if (address.getFamily() != sockets_[0].addressFamily_) {
      throw TTransportException(TTransportException::BAD_ARGS,
                                "Attempted to bind address to socket with "
                                "different address family");
    }
    fd = sockets_[0].socket_;
  } else {
    throw TTransportException(TTransportException::BAD_ARGS,
                              "Attempted to bind to multiple fds");
  }

  // Bind to the socket
  if (::bind(fd, address.getAddress(), address.getActualSize()) != 0) {
    if (sockets_.size() == 0) {
      ::close(fd);
    }
    throw TTransportException(TTransportException::COULD_NOT_BIND,
                              "failed to bind to async server socket: " +
                              address.describe(),
                              errno);
  }

  // Record the address family that we are using,
  // so we know how much address space we need to record accepted addresses.

  // If we just created this socket, update the TEventHandler and set socket_
  if (sockets_.size() == 0) {
    sockets_.push_back(
      ServerEventHandler(eventBase_, fd, this, address.getFamily()));
    sockets_[0].changeHandlerFD(fd);
  }
}

void TAsyncServerSocket::bind(uint16_t port) {
  struct addrinfo hints, *res, *res0;
  char sport[sizeof("65536")];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
  snprintf(sport, sizeof(sport), "%u", port);

  if (getaddrinfo(nullptr, sport, &hints, &res0)) {
    throw TTransportException(TTransportException::BAD_ARGS,
                              "Attempted to bind address to socket with "
                              "bad getaddrinfo");
  }

  folly::ScopeGuard guard = folly::makeGuard([&]{
      freeaddrinfo(res0);
    });
  DCHECK(&guard);

  for (res = res0; res; res = res->ai_next) {
    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    CHECK(s);

    try {
      setupSocket(s);
    } catch (...) {
      ::close(s);
      throw;
    }

    if (res->ai_family == AF_INET6) {
      int v6only = 1;
      CHECK(0 == setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY,
                            &v6only, sizeof(v6only)));
    }

    TSocketAddress address;
    address.setFromLocalAddress(s);

    sockets_.push_back(
      ServerEventHandler(eventBase_, s, this, address.getFamily()));

    // Bind to the socket
    if (::bind(s, res->ai_addr, res->ai_addrlen) != 0) {
      throw TTransportException(
        TTransportException::COULD_NOT_BIND,
        "failed to bind to async server socket for port",
        errno);
    }
  }
}

void TAsyncServerSocket::listen(int backlog) {
  assert(eventBase_ == nullptr || eventBase_->isInEventBaseThread());

  // Start listening
  for (auto& handler : sockets_) {
    if (::listen(handler.socket_, backlog) == -1) {
      throw TTransportException(TTransportException::NOT_OPEN,
                                "failed to listen on async server socket",
                                errno);
    }
  }
}

void TAsyncServerSocket::getAddress(TSocketAddress* addressReturn) const {
  CHECK(sockets_.size() >= 1);
  if (sockets_.size() > 1) {
    VLOG(2) << "Warning: getAddress can return multiple addresses, " <<
      "but getAddress was called, so only returning the first";
  }
  addressReturn->setFromLocalAddress(sockets_[0].socket_);
}

void TAsyncServerSocket::addAcceptCallback(AcceptCallback *callback,
                                           TEventBase *eventBase,
                                           uint32_t maxAtOnce) {
  assert(eventBase_ == nullptr || eventBase_->isInEventBaseThread());

  // If this is the first accept callback and we are supposed to be accepting,
  // start accepting once the callback is installed.
  bool runStartAccepting = accepting_ && callbacks_.empty();

  callbacks_.push_back(CallbackInfo(callback, eventBase));

  if (!eventBase) {
    eventBase = eventBase_; // Run in TAsyncServerSocket's eventbase
  }

  // Start the remote acceptor.
  //
  // It would be nice if we could avoid starting the remote acceptor if
  // eventBase == eventBase_.  However, that would cause issues if
  // detachEventBase() and attachEventBase() were ever used to change the
  // primary TEventBase for the server socket.  Therefore we require the caller
  // to specify a nullptr TEventBase if they want to ensure that the callback is
  // always invoked in the primary TEventBase, and to be able to invoke that
  // callback more efficiently without having to use a notification queue.
  RemoteAcceptor* acceptor = nullptr;
  try {
    acceptor = new RemoteAcceptor(callback);
    acceptor->start(eventBase, maxAtOnce, maxNumMsgsInQueue_);
  } catch (...) {
    callbacks_.pop_back();
    delete acceptor;
    throw;
  }
  callbacks_.back().consumer = acceptor;

  // If this is the first accept callback and we are supposed to be accepting,
  // start accepting.
  if (runStartAccepting) {
    startAccepting();
  }
}

void TAsyncServerSocket::removeAcceptCallback(AcceptCallback *callback,
                                              TEventBase *eventBase) {
  assert(eventBase_ == nullptr || eventBase_->isInEventBaseThread());

  // Find the matching AcceptCallback.
  // We just do a simple linear search; we don't expect removeAcceptCallback()
  // to be called frequently, and we expect there to only be a small number of
  // callbacks anyway.
  std::vector<CallbackInfo>::iterator it = callbacks_.begin();
  uint32_t n = 0;
  while (true) {
    if (it == callbacks_.end()) {
      throw TLibraryException("TAsyncServerSocket::removeAcceptCallback(): "
                              "accept callback not found");
    }
    if (it->callback == callback && it->eventBase == eventBase) {
      break;
    }
    ++it;
    ++n;
  }

  // Remove this callback from callbacks_.
  //
  // Do this before invoking the acceptStopped() callback, in case
  // acceptStopped() invokes one of our methods that examines callbacks_.
  //
  // Save a copy of the CallbackInfo first.
  CallbackInfo info(*it);
  callbacks_.erase(it);
  if (n < callbackIndex_) {
    // We removed an element before callbackIndex_.  Move callbackIndex_ back
    // one step, since things after n have been shifted back by 1.
    --callbackIndex_;
  } else {
    // We removed something at or after callbackIndex_.
    // If we removed the last element and callbackIndex_ was pointing at it,
    // we need to reset callbackIndex_ to 0.
    if (callbackIndex_ >= callbacks_.size()) {
      callbackIndex_ = 0;
    }
  }

  info.consumer->stop(info.eventBase, info.callback);

  // If we are supposed to be accepting but the last accept callback
  // was removed, unregister for events until a callback is added.
  if (accepting_ && callbacks_.empty()) {
    for (auto& handler : sockets_) {
      handler.unregisterHandler();
    }
  }
}

void TAsyncServerSocket::startAccepting() {
  assert(eventBase_ == nullptr || eventBase_->isInEventBaseThread());

  accepting_ = true;
  if (callbacks_.empty()) {
    // We can't actually begin accepting if no callbacks are defined.
    // Wait until a callback is added to start accepting.
    return;
  }

  for (auto& handler : sockets_) {
    if (!handler.registerHandler(
          TEventHandler::READ | TEventHandler::PERSIST)) {
      throw TTransportException(TTransportException::NOT_OPEN,
                                "failed to register for accept events");
    }
  }
}

void TAsyncServerSocket::pauseAccepting() {
  assert(eventBase_ == nullptr || eventBase_->isInEventBaseThread());
  accepting_ = false;
  for (auto& handler : sockets_) {
   handler. unregisterHandler();
  }

  // If we were in the accept backoff state, disable the backoff timeout
  if (backoffTimeout_) {
    backoffTimeout_->cancelTimeout();
  }
}

int TAsyncServerSocket::createSocket(int family) {
  int fd = socket(family, SOCK_STREAM, 0);
  if (fd == -1) {
    throw TTransportException(TTransportException::NOT_OPEN,
                              "error creating async server socket", errno);
  }

  try {
    setupSocket(fd);
  } catch (...) {
    ::close(fd);
    throw;
  }
  return fd;
}

void TAsyncServerSocket::setupSocket(int fd) {
  // Get the address family
  TSocketAddress address;
  address.setFromLocalAddress(fd);
  auto family = address.getFamily();

  // Put the socket in non-blocking mode
  if (fcntl(fd, F_SETFL, O_NONBLOCK) != 0) {
    throw TTransportException(TTransportException::NOT_OPEN,
                              "failed to put socket in non-blocking mode",
                              errno);
  }

  // Set reuseaddr to avoid 2MSL delay on server restart
  int one = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0) {
    // This isn't a fatal error; just log an error message and continue
    T_ERROR("failed to set SO_REUSEADDR on async server socket: %s",
            strerror(errno));
  }

  // Set keepalive as desired
  int zero = 0;
  if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
                 (keepAliveEnabled_) ? &one : &zero, sizeof(int)) != 0) {
    T_ERROR("failed to set SO_KEEPALIVE on async server socket: %s",
            strerror(errno));
  }

  // Setup FD_CLOEXEC flag
  if (closeOnExec_ &&
      (-1 == apache::thrift::util::setCloseOnExec(fd, closeOnExec_))) {
    T_ERROR("failed to set FD_CLOEXEC on async server socket: %s",
            strerror(errno));
  }

  // Set TCP nodelay if available, MAC OS X Hack
  // See http://lists.danga.com/pipermail/memcached/2005-March/001240.html
#ifndef TCP_NOPUSH
  if (family != AF_UNIX) {
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) != 0) {
      // This isn't a fatal error; just log an error message and continue
      T_ERROR("failed to set TCP_NODELAY on async server socket: %s",
              strerror(errno));
    }
  }
#endif

  if (shutdownSocketSet_) {
    shutdownSocketSet_->add(fd);
  }
}

void TAsyncServerSocket::handlerReady(
  uint16_t events, int fd, sa_family_t addressFamily) noexcept {
  assert(!callbacks_.empty());
  DestructorGuard dg(this);

  // Only accept up to maxAcceptAtOnce_ connections at a time,
  // to avoid starving other I/O handlers using this TEventBase.
  for (uint32_t n = 0; n < maxAcceptAtOnce_; ++n) {
    TSocketAddress address;
    socklen_t addrLen;

    struct sockaddr *addrStorage = address.getMutableAddress(
        addressFamily, &addrLen);

    // Accept a new client socket
    int clientSocket = accept(fd, addrStorage, &addrLen);
    address.addressUpdated(addressFamily, addrLen);

    int64_t nowMs = concurrency::Util::currentTime();
    int64_t timeSinceLastAccept = std::max(0L, nowMs - lastAccepTimestamp_);
    lastAccepTimestamp_ = nowMs;
    if (acceptRate_ < 1) {
      acceptRate_ *= 1 + acceptRateAdjustSpeed_ * timeSinceLastAccept;
      if (acceptRate_ >= 1) {
        acceptRate_ = 1;
        GlobalOutput.printf("Change acceptRate to %lf", acceptRate_);
      } else if (rand() > acceptRate_ * RAND_MAX) {
        ++numDroppedConnections_;
        if (clientSocket >= 0) {
          ::close(clientSocket);
        }
        continue;
      }
    }

    if (clientSocket < 0) {
      if (errno == EAGAIN) {
        // No more sockets to accept right now.
        // Check for this code first, since it's the most common.
        return;
      } else if (errno == EMFILE || errno == ENFILE) {
        // We're out of file descriptors.  Perhaps we're accepting connections
        // too quickly. Pause accepting briefly to back off and give the server
        // a chance to recover.
        T_ERROR("accept failed: out of file descriptors; entering accept "
                "back-off state");
        enterBackoff();

        // Dispatch the error message
        dispatchError("accept() failed", errno);
      } else {
        dispatchError("accept() failed", errno);
      }
      return;
    }

    // Explicitly set the new connection to non-blocking mode
    if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) != 0) {
      ::close(clientSocket);
      dispatchError("failed to set accepted socket to non-blocking mode",
                    errno);
      return;
    }

    // Inform the callback about the new connection
    dispatchSocket(clientSocket, std::move(address));

    // If we aren't accepting any more, break out of the loop
    if (!accepting_ || callbacks_.empty()) {
      break;
    }
  }
}

void TAsyncServerSocket::dispatchSocket(int socket,
                                        TSocketAddress&& address) {
  uint32_t startingIndex = callbackIndex_;

  // Short circuit if the callback is in the primary TEventBase thread
  CallbackInfo *info = nextCallback();
  if (info->eventBase == nullptr) {
    info->callback->connectionAccepted(socket, address);
    return;
  }

  // Create a message to send over the notification queue
  QueueMessage msg;
  msg.type = MessageType::MSG_NEW_CONN;
  msg.address = std::move(address);
  msg.fd = socket;

  // Loop until we find a free queue to write to
  while (true) {
    if (info->consumer->getQueue()->tryPutMessageNoThrow(std::move(msg))) {
      // Success! return.
      return;
   }

    // We couldn't add to queue.  Fall through to below

    ++numDroppedConnections_;
    if (acceptRateAdjustSpeed_ > 0) {
      // aggressively decrease accept rate when in trouble
      static const double kAcceptRateDecreaseSpeed = 0.1;
      acceptRate_ *= 1 - kAcceptRateDecreaseSpeed;
      GlobalOutput.printf("Change acceptRate to %lf", acceptRate_);
    }


    if (callbackIndex_ == startingIndex) {
      // The notification queue was full
      // We can't really do anything at this point other than close the socket.
      //
      // This should only happen if a user's service is behaving extremely
      // badly and none of the TEventBase threads are looping fast enough to
      // process the incoming connections.  If the service is overloaded, it
      // should use pauseAccepting() to temporarily back off accepting new
      // connections, before they reach the point where their threads can't
      // even accept new messages.
      T_ERROR("failed to dispatch newly accepted socket: all accept callback "
              "queues are full");
      ::close(socket);
      return;
    }

    info = nextCallback();
  }
}

void TAsyncServerSocket::dispatchError(const char *msgstr, int errnoValue) {
  uint32_t startingIndex = callbackIndex_;
  CallbackInfo *info = nextCallback();

  // Create a message to send over the notification queue
  QueueMessage msg;
  msg.type = MessageType::MSG_ERROR;
  msg.err = errnoValue;
  msg.msg = std::move(msgstr);

  while (true) {
    // Short circuit if the callback is in the primary TEventBase thread
    if (info->eventBase == nullptr) {
      TTransportException ex(TTransportException::INTERNAL_ERROR,
                             msgstr, errnoValue);
      info->callback->acceptError(ex);
      return;
    }

    if (info->consumer->getQueue()->tryPutMessageNoThrow(std::move(msg))) {
      return;
    }
    // Fall through and try another callback

    if (callbackIndex_ == startingIndex) {
      // The notification queues for all of the callbacks were full.
      // We can't really do anything at this point.
      T_ERROR("failed to dispatch accept error: all accept callback "
              "queues are full: error msg: %s (errno=%d)",
              msg.msg.c_str(), errnoValue);
      return;
    }
    info = nextCallback();
  }
}

void TAsyncServerSocket::enterBackoff() {
  // If this is the first time we have entered the backoff state,
  // allocate backoffTimeout_.
  if (backoffTimeout_ == nullptr) {
    try {
      backoffTimeout_ = new BackoffTimeout(this);
    } catch (const std::bad_alloc& ex) {
      // Man, we couldn't even allocate the timer to re-enable accepts.
      // We must be in pretty bad shape.  Don't pause accepting for now,
      // since we won't be able to re-enable ourselves later.
      T_ERROR("failed to allocate TAsyncServerSocket backoff timer; unable to "
              "temporarly pause accepting");
      return;
    }
  }

  // For now, we simply pause accepting for 1 second.
  //
  // We could add some smarter backoff calculation here in the future.  (e.g.,
  // start sleeping for longer if we keep hitting the backoff frequently.)
  // Typically the user needs to figure out why the server is overloaded and
  // fix it in some other way, though.  The backoff timer is just a simple
  // mechanism to try and give the connection processing code a little bit of
  // breathing room to catch up, and to avoid just spinning and failing to
  // accept over and over again.
  const uint32_t timeoutMS = 1000;
  if (!backoffTimeout_->scheduleTimeout(timeoutMS)) {
    T_ERROR("failed to schedule TAsyncServerSocket backoff timer; unable to "
            "temporarly pause accepting");
    return;
  }

  // The backoff timer is scheduled to re-enable accepts.
  // Go ahead and disable accepts for now.  We leave accepting_ set to true,
  // since that tracks the desired state requested by the user.
  for (auto& handler : sockets_) {
    handler.unregisterHandler();
  }
}

void TAsyncServerSocket::backoffTimeoutExpired() {
  // accepting_ should still be true.
  // If pauseAccepting() was called while in the backoff state it will cancel
  // the backoff timeout.
  assert(accepting_);
  // We can't be detached from the TEventBase without being paused
  assert(eventBase_ != nullptr && eventBase_->isInEventBaseThread());

  // If all of the callbacks were removed, we shouldn't re-enable accepts
  if (callbacks_.empty()) {
    return;
  }

  // Register the handler.
  for (auto& handler : sockets_) {
    if (!handler.registerHandler(
          TEventHandler::READ | TEventHandler::PERSIST)) {
      // We're hosed.  We could just re-schedule backoffTimeout_ to
      // re-try again after a little bit.  However, we don't want to
      // loop retrying forever if we can't re-enable accepts.  Just
      // abort the entire program in this state; things are really bad
      // and restarting the entire server is probably the best remedy.
      T_ERROR("failed to re-enable TAsyncServerSocket accepts after backoff; "
              "crashing now");
      abort();
    }
  }
}

}}} // apache::thrift::async
