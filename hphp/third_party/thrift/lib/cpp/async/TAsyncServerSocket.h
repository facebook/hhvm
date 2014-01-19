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
#ifndef HPHP_THRIFT_ASYNC_TASYNCSERVERSOCKET_H
#define HPHP_THRIFT_ASYNC_TASYNCSERVERSOCKET_H 1

#include "thrift/lib/cpp/ShutdownSocketSet.h"
#include "thrift/lib/cpp/thrift_config.h"
#include "thrift/lib/cpp/async/TDelayedDestruction.h"
#include "thrift/lib/cpp/async/TEventHandler.h"
#include "thrift/lib/cpp/async/TNotificationQueue.h"
#include "thrift/lib/cpp/transport/TSocketAddress.h"
#include <memory>
#include <exception>
#include <vector>
#include <limits.h>
#include <stddef.h>
#include <memory>
#include <sys/socket.h>

namespace apache { namespace thrift {

namespace async {

/**
 * A listening socket that asynchronously informs a callback whenever a new
 * connection has been accepted.
 *
 * Unlike most async interfaces that always invoke their callback in the same
 * TEventBase thread, TAsyncServerSocket is unusual in that it can distribute
 * the callbacks across multiple TEventBase threads.
 *
 * This supports a common use case for network servers to distribute incoming
 * connections across a number of TEventBase threads.  (Servers typically run
 * with one TEventBase thread per CPU.)
 *
 * Despite being able to invoke callbacks in multiple TEventBase threads,
 * TAsyncServerSocket still has one "primary" TEventBase.  Operations that
 * modify the TAsyncServerSocket state may only be performed from the primary
 * TEventBase thread.
 */
class TAsyncServerSocket : public TDelayedDestruction {
 public:
  typedef std::unique_ptr<TAsyncServerSocket, Destructor> UniquePtr;

  class AcceptCallback {
   public:
    virtual ~AcceptCallback() {}

    /**
     * connectionAccepted() is called whenever a new client connection is
     * received.
     *
     * The AcceptCallback will remain installed after connectionAccepted()
     * returns.
     *
     * @param fd          The newly accepted client socket.  The AcceptCallback
     *                    assumes ownership of this socket, and is responsible
     *                    for closing it when done.  The newly accepted file
     *                    descriptor will have already been put into
     *                    non-blocking mode.
     * @param clientAddr  A reference to a TSocketAddress struct containing the
     *                    client's address.  This struct is only guaranteed to
     *                    remain valid until connectionAccepted() returns.
     */
    virtual void connectionAccepted(int fd,
                                    const transport::TSocketAddress& clientAddr)
      noexcept = 0;

    /**
     * acceptError() is called if an error occurs while accepting.
     *
     * The AcceptCallback will remain installed even after an accept error,
     * as the errors are typically somewhat transient, such as being out of
     * file descriptors.  The server socket must be explicitly stopped if you
     * wish to stop accepting after an error.
     *
     * @param ex  An exception representing the error.
     */
    virtual void acceptError(const std::exception& ex) noexcept = 0;

    /**
     * acceptStarted() will be called in the callback's TEventBase thread
     * after this callback has been added to the TAsyncServerSocket.
     *
     * acceptStarted() will be called before any calls to connectionAccepted()
     * or acceptError() are made on this callback.
     *
     * acceptStarted() makes it easier for callbacks to perform initialization
     * inside the callback thread.  (The call to addAcceptCallback() must
     * always be made from the TAsyncServerSocket's primary TEventBase thread.
     * acceptStarted() provides a hook that will always be invoked in the
     * callback's thread.)
     *
     * Note that the call to acceptStarted() is made once the callback is
     * added, regardless of whether or not the TAsyncServerSocket is actually
     * accepting at the moment.  acceptStarted() will be called even if the
     * TAsyncServerSocket is paused when the callback is added (including if
     * the initial call to startAccepting() on the TAsyncServerSocket has not
     * been made yet).
     */
    virtual void acceptStarted() noexcept {}

    /**
     * acceptStopped() will be called when this AcceptCallback is removed from
     * the TAsyncServerSocket, or when the TAsyncServerSocket is destroyed,
     * whichever occurs first.
     *
     * No more calls to connectionAccepted() or acceptError() will be made
     * after acceptStopped() is invoked.
     */
    virtual void acceptStopped() noexcept {}
  };

  static const uint32_t kDefaultMaxAcceptAtOnce = 30;
  static const uint32_t kDefaultCallbackAcceptAtOnce = 5;
  static const uint32_t kDefaultMaxMessagesInQueue = 0;
  /**
   * Create a new TAsyncServerSocket with the specified TEventBase.
   *
   * @param eventBase  The TEventBase to use for driving the asynchronous I/O.
   *                   If this parameter is nullptr, attachEventBase() must be
   *                   called before this socket can begin accepting
   *                   connections.
   */
  explicit TAsyncServerSocket(TEventBase* eventBase = nullptr);

  /**
   * Helper function to create a shared_ptr<TAsyncServerSocket>.
   *
   * This passes in the correct destructor object, since TAsyncServerSocket's
   * destructor is protected and cannot be invoked directly.
   */
  static std::shared_ptr<TAsyncServerSocket>
  newSocket(TEventBase* evb = nullptr) {
    return std::shared_ptr<TAsyncServerSocket>(new TAsyncServerSocket(evb),
                                                 Destructor());
  }

  void setShutdownSocketSet(ShutdownSocketSet* newSS);

  /**
   * Destroy the socket.
   *
   * TAsyncServerSocket::destroy() must be called to destroy the socket.
   * The normal destructor is private, and should not be invoked directly.
   * This prevents callers from deleting a TAsyncServerSocket while it is
   * invoking a callback.
   *
   * destroy() must be invoked from the socket's primary TEventBase thread.
   *
   * If there are AcceptCallbacks still installed when destroy() is called,
   * acceptStopped() will be called on these callbacks to notify them that
   * accepting has stopped.  Accept callbacks being driven by other TEventBase
   * threads may continue to receive new accept callbacks for a brief period of
   * time after destroy() returns.  They will not receive any more callback
   * invocations once acceptStopped() is invoked.
   */
  virtual void destroy();

  /**
   * Attach this TAsyncServerSocket to its primary TEventBase.
   *
   * This may only be called if the TAsyncServerSocket is not already attached
   * to a TEventBase.  The TAsyncServerSocket must be attached to a TEventBase
   * before it can begin accepting connections.
   */
  void attachEventBase(TEventBase *eventBase);

  /**
   * Detach the TAsyncServerSocket from its primary TEventBase.
   *
   * detachEventBase() may only be called if the TAsyncServerSocket is not
   * currently accepting connections.
   */
  void detachEventBase();

  /**
   * Get the TEventBase used by this socket.
   */
  TEventBase* getEventBase() const {
    return eventBase_;
  }

  /**
   * Create a TAsyncServerSocket from an existing socket file descriptor.
   *
   * useExistingSocket() will cause the TAsyncServerSocket to take ownership of
   * the specified file descriptor, and use it to listen for new connections.
   * The TAsyncServerSocket will close the file descriptor when it is
   * destroyed.
   *
   * useExistingSocket() must be called before bind() or listen().
   *
   * The supplied file descriptor will automatically be put into non-blocking
   * mode.  The caller may have already directly called bind() and possibly
   * listen on the file descriptor.  If so the caller should skip calling the
   * corresponding TAsyncServerSocket::bind() and listen() methods.
   *
   * On error a TTransportException will be thrown and the caller will retain
   * ownership of the file descriptor.
   */
  void useExistingSocket(int fd);

  /**
   * Return the underlying file descriptor
   */
  std::vector<int> getSockets() const {
    std::vector<int> sockets;
    for (auto& handler : sockets_) {
      sockets.push_back(handler.socket_);
    }
    return sockets;
  }

  /**
   * Backwards compatible getSocket, warns if > 1 socket
   */
  int getSocket() const {
    if (sockets_.size() > 1) {
      VLOG(2) << "Warning: getSocket can return multiple fds, " <<
        "but getSockets was not called, so only returning the first";
    }
    if (sockets_.size() == 0) {
      return -1;
    } else {
      return sockets_[0].socket_;
    }
  }

  /**
   * Bind to the specified address.
   *
   * This must be called from the primary TEventBase thread.
   *
   * Throws TTransportException on error.
   */
  virtual void bind(const transport::TSocketAddress& address);

  /**
   * Bind to the specified port.
   *
   * This must be called from the primary TEventBase thread.
   *
   * Throws TTransportException on error.
   */
  virtual void bind(uint16_t port);

  /**
   * Get the local address to which the socket is bound.
   *
   * Throws TTransportException on error.
   */
  void getAddress(transport::TSocketAddress* addressReturn) const;

  /**
   * Begin listening for connections.
   *
   * This calls ::listen() with the specified backlog.
   *
   * Once listen() is invoked the socket will actually be open so that remote
   * clients may establish connections.  (Clients that attempt to connect
   * before listen() is called will receive a connection refused error.)
   *
   * At least one callback must be set and startAccepting() must be called to
   * actually begin notifying the accept callbacks of newly accepted
   * connections.  The backlog parameter controls how many connections the
   * kernel will accept and buffer internally while the accept callbacks are
   * paused (or if accepting is enabled but the callbacks cannot keep up).
   *
   * bind() must be called before calling listen().
   * listen() must be called from the primary TEventBase thread.
   *
   * Throws TTransportException on error.
   */
  virtual void listen(int backlog);

  /**
   * Add an AcceptCallback.
   *
   * When a new socket is accepted, one of the AcceptCallbacks will be invoked
   * with the new socket.  The AcceptCallbacks are invoked in a round-robin
   * fashion.  This allows the accepted sockets to distributed among a pool of
   * threads, each running its own TEventBase object.  This is a common model,
   * since most asynchronous-style servers typically run one TEventBase thread
   * per CPU.
   *
   * The TEventBase object associated with each AcceptCallback must be running
   * its loop.  If the TEventBase loop is not running, sockets will still be
   * scheduled for the callback, but the callback cannot actually get invoked
   * until the loop runs.
   *
   * This method must be invoked from the TAsyncServerSocket's primary
   * TEventBase thread.
   *
   * Note that startAccepting() must be called on the TAsyncServerSocket to
   * cause it to actually start accepting sockets once callbacks have been
   * installed.
   *
   * @param callback   The callback to invoke.
   * @param eventBase  The TEventBase to use to invoke the callback.  This
   *     parameter may be nullptr, in which case the callback will be invoked in
   *     the TAsyncServerSocket's primary TEventBase.
   * @param maxAtOnce  The maximum number of connections to accept in this
   *                   callback on a single iteration of the event base loop.
   *                   This only takes effect when eventBase is non-nullptr.
   *                   When using a nullptr eventBase for the callback, the
   *                   setMaxAcceptAtOnce() method controls how many
   *                   connections the main event base will accept at once.
   */
  virtual void addAcceptCallback(
    AcceptCallback *callback,
    TEventBase *eventBase,
    uint32_t maxAtOnce = kDefaultCallbackAcceptAtOnce);

  /**
   * Remove an AcceptCallback.
   *
   * This allows a single AcceptCallback to be removed from the round-robin
   * pool.
   *
   * This method must be invoked from the TAsyncServerSocket's primary
   * TEventBase thread.  Use TEventBase::runInEventBaseThread() to schedule the
   * operation in the correct TEventBase if your code is not in the server
   * socket's primary TEventBase.
   *
   * Given that the accept callback is being driven by a different TEventBase,
   * the AcceptCallback may continue to be invoked for a short period of time
   * after removeAcceptCallback() returns in this thread.  Once the other
   * TEventBase thread receives the notification to stop, it will call
   * acceptStopped() on the callback to inform it that it is fully stopped and
   * will not receive any new sockets.
   *
   * If the last accept callback is removed while the socket is accepting,
   * the socket will implicitly pause accepting.  If a callback is later added,
   * it will resume accepting immediately, without requiring startAccepting()
   * to be invoked.
   *
   * @param callback   The callback to uninstall.
   * @param eventBase  The TEventBase associated with this callback.  This must
   *     be the same TEventBase that was used when the callback was installed
   *     with addAcceptCallback().
   */
  void removeAcceptCallback(AcceptCallback *callback, TEventBase *eventBase);

  /**
   * Begin accepting connctions on this socket.
   *
   * bind() and listen() must be called before calling startAccepting().
   *
   * When a TAsyncServerSocket is initially created, it will not begin
   * accepting connections until at least one callback has been added and
   * startAccepting() has been called.  startAccepting() can also be used to
   * resume accepting connections after a call to pauseAccepting().
   *
   * If startAccepting() is called when there are no accept callbacks
   * installed, the socket will not actually begin accepting until an accept
   * callback is added.
   *
   * This method may only be called from the primary TEventBase thread.
   */
  virtual void startAccepting();

  /**
   * Pause accepting connections.
   *
   * startAccepting() may be called to resume accepting.
   *
   * This method may only be called from the primary TEventBase thread.
   * If there are AcceptCallbacks being driven by other TEventBase threads they
   * may continue to receive callbacks for a short period of time after
   * pauseAccepting() returns.
   *
   * Unlike removeAcceptCallback() or destroy(), acceptStopped() will not be
   * called on the AcceptCallback objects simply due to a temporary pause.  If
   * the server socket is later destroyed while paused, acceptStopped() will be
   * called all of the installed AcceptCallbacks.
   */
  void pauseAccepting();

  /**
   * Get the maximum number of connections that will be accepted each time
   * around the event loop.
   */
  uint32_t getMaxAcceptAtOnce() const {
    return maxAcceptAtOnce_;
  }

  /**
   * Set the maximum number of connections that will be accepted each time
   * around the event loop.
   *
   * This provides a very coarse-grained way of controlling how fast the
   * TAsyncServerSocket will accept connections.  If you find that when your
   * server is overloaded TAsyncServerSocket accepts connections more quickly
   * than your code can process them, you can try lowering this number so that
   * fewer connections will be accepted each event loop iteration.
   *
   * For more explicit control over the accept rate, you can also use
   * pauseAccepting() to temporarily pause accepting when your server is
   * overloaded, and then use startAccepting() later to resume accepting.
   */
  void setMaxAcceptAtOnce(uint32_t numConns) {
    maxAcceptAtOnce_ = numConns;
  }

  /**
   * Get the maximum number of unprocessed messages which a NotificationQueue
   * can hold.
   */
  uint32_t getMaxNumMessagesInQueue() const {
    return maxNumMsgsInQueue_;
  }

  /**
   * Set the maximum number of unprocessed messages in NotificationQueue.
   * No new message will be sent to that NotificationQueue if there are more
   * than such number of unprocessed messages in that queue.
   *
   * Only works if called before addAcceptCallback.
   */
  void setMaxNumMessagesInQueue(uint32_t num) {
    maxNumMsgsInQueue_ = num;
  }

  /**
   * Get the speed of adjusting connection accept rate.
   */
  double getAcceptRateAdjustSpeed() const {
    return acceptRateAdjustSpeed_;
  }

  /**
   * Set the speed of adjusting connection accept rate.
   */
  void setAcceptRateAdjustSpeed(double speed) {
    acceptRateAdjustSpeed_ = speed;
  }

  /**
   * Get the number of connections dropped by the TAsyncServerSocket
   */
  uint64_t getNumDroppedConnections() const {
    return numDroppedConnections_;
  }

  /**
   * Set whether or not SO_KEEPALIVE should be enabled on the server socket
   * (and thus on all subsequently-accepted connections). By default, keepalive
   * is enabled.
   *
   * Note that TCP keepalive usually only kicks in after the connection has
   * been idle for several hours. Applications should almost always have their
   * own, shorter idle timeout.
   */
  void setKeepAliveEnabled(bool enabled) {
    keepAliveEnabled_ = enabled;

    for (auto& handler : sockets_) {
      if (handler.socket_ < 0) {
        continue;
      }

      int val = (enabled) ? 1 : 0;
      if (setsockopt(handler.socket_, SOL_SOCKET,
                     SO_KEEPALIVE, &val, sizeof(val)) != 0) {
        T_ERROR("failed to set SO_KEEPALIVE on async server socket: %s",
                strerror(errno));
      }
    }
  }

  /**
   * Get whether or not SO_KEEPALIVE is enabled on the server socket.
   */
  bool getKeepAliveEnabled() const {
    return keepAliveEnabled_;
  }

  /**
   * Set whether or not the socket should close during exec() (FD_CLOEXEC). By
   * default, this is enabled
   */
  void setCloseOnExec(bool closeOnExec) {
    closeOnExec_ = closeOnExec;
  }

  /**
   * Get whether or not FD_CLOEXEC is enabled on the server socket.
   */
  bool getCloseOnExec() const {
    return closeOnExec_;
  }

 protected:
  /**
   * Protected destructor.
   *
   * Invoke destroy() instead to destroy the TAsyncServerSocket.
   */
  virtual ~TAsyncServerSocket();

 private:
  enum class MessageType {
    MSG_NEW_CONN = 0,
    MSG_ERROR = 1
  };

  struct QueueMessage {
    MessageType type;
    int fd;
    int err;
    transport::TSocketAddress address;
    std::string msg;
  };

  /**
   * A class to receive notifications to invoke AcceptCallback objects
   * in other TEventBase threads.
   *
   * A RemoteAcceptor object is created for each AcceptCallback that
   * is installed in a separate TEventBase thread.  The RemoteAcceptor
   * receives notification of new sockets via a TNotificationQueue,
   * and then invokes the AcceptCallback.
   */
  class RemoteAcceptor
      : private TNotificationQueue<QueueMessage>::Consumer {
  public:
    explicit RemoteAcceptor(AcceptCallback *callback)
      : callback_(callback) {}

    ~RemoteAcceptor() {}

    void start(TEventBase *eventBase, uint32_t maxAtOnce, uint32_t maxInQueue);
    void stop(TEventBase* eventBase, AcceptCallback* callback);

    virtual void messageAvailable(QueueMessage&& message);

    TNotificationQueue<QueueMessage>* getQueue() {
      return &queue_;
    }

  private:
    AcceptCallback *callback_;

    TNotificationQueue<QueueMessage> queue_;
  };

  /**
   * A struct to keep track of the callbacks associated with this server
   * socket.
   */
  struct CallbackInfo {
    CallbackInfo(AcceptCallback *callback, TEventBase *eventBase)
      : callback(callback),
        eventBase(eventBase),
        consumer(nullptr) {}

    AcceptCallback *callback;
    TEventBase *eventBase;

    RemoteAcceptor* consumer;
  };

  class BackoffTimeout;

  virtual void handlerReady(
    uint16_t events, int socket, sa_family_t family) noexcept;

  int createSocket(int family);
  void setupSocket(int fd);
  void dispatchSocket(int socket, transport::TSocketAddress&& address);
  void dispatchError(const char *msg, int errnoValue);
  void enterBackoff();
  void backoffTimeoutExpired();

  CallbackInfo* nextCallback() {
    CallbackInfo* info = &callbacks_[callbackIndex_];

    ++callbackIndex_;
    if (callbackIndex_ >= callbacks_.size()) {
      callbackIndex_ = 0;
    }

    return info;
  }

  struct ServerEventHandler : public TEventHandler {
    ServerEventHandler(TEventBase* eventBase, int socket,
                       TAsyncServerSocket* parent,
                      sa_family_t addressFamily)
        : TEventHandler(eventBase, socket)
        , eventBase_(eventBase)
        , socket_(socket)
        , parent_(parent)
        , addressFamily_(addressFamily) {}

    ServerEventHandler(const ServerEventHandler& other)
    : TEventHandler(other.eventBase_, other.socket_)
    , eventBase_(other.eventBase_)
    , socket_(other.socket_)
    , parent_(other.parent_)
    , addressFamily_(other.addressFamily_) {}

    ServerEventHandler& operator=(
        const ServerEventHandler& other) {
      if (this != &other) {
        eventBase_ = other.eventBase_;
        socket_ = other.socket_;
        parent_ = other.parent_;
        addressFamily_ = other.addressFamily_;

        detachEventBase();
        attachEventBase(other.eventBase_);
        changeHandlerFD(other.socket_);
      }
      return *this;
    }

    // Inherited from TEventHandler
    virtual void handlerReady(uint16_t events) noexcept {
      parent_->handlerReady(events, socket_, addressFamily_);
    }

    TEventBase* eventBase_;
    int socket_;
    TAsyncServerSocket* parent_;
    sa_family_t addressFamily_;
  };

  TEventBase *eventBase_;
  std::vector<ServerEventHandler> sockets_;
  bool accepting_;
  uint32_t maxAcceptAtOnce_;
  uint32_t maxNumMsgsInQueue_;
  double acceptRateAdjustSpeed_;  //0 to disable auto adjust
  double acceptRate_;
  int64_t lastAccepTimestamp_;  // milliseconds
  uint64_t numDroppedConnections_;
  uint32_t callbackIndex_;
  BackoffTimeout *backoffTimeout_;
  std::vector<CallbackInfo> callbacks_;
  bool keepAliveEnabled_;
  bool closeOnExec_;
  ShutdownSocketSet* shutdownSocketSet_;
};

}}} // apache::thrift::async

#endif // HPHP_THRIFT_ASYNC_TASYNCSERVERSOCKET_H
