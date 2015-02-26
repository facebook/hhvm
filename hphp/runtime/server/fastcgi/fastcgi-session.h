/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_SESSION_H_
#define incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_SESSION_H_

#include "hphp/runtime/server/fastcgi/fastcgi-protocol.h"
#include "hphp/runtime/server/fastcgi/fastcgi-transport.h"
#include "hphp/runtime/server/fastcgi/fastcgi-worker.h"
#include "hphp/util/job-queue.h"

#include <folly/io/IOBuf.h>
#include <folly/io/Cursor.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/wangle/acceptor/ManagedConnection.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

namespace fcgi {
/*
 * KVParser will process a key value stream from a FastCGI webserver. When
 * used for processing PARAMS data it should be reset between requests.
 */
struct KVParser {
  // a k/v pair is available
  bool ready() { return m_phase == Phase::READY; }

  // no more k/v pairs will ever be available
  bool done() { return m_phase == Phase::COMPLETE; }

  void reset(); // reset to accepting state

  // Consume will process new data and return false if a malformed stream is
  // detected.
  bool consume(std::unique_ptr<folly::IOBuf> chain);

  // reads a k/v pair and returns to the accepting state. Returns (key, value)
  // Precondition: ready() == true
  std::tuple<
    std::unique_ptr<folly::IOBuf>,
    std::unique_ptr<folly::IOBuf>
  > readNext();

private:
  /* KVParser is a state-machine for reading key-value pairs in streams. The
   * phase indicates that we are in the process of accepting information about
   * a key or a value for PARAMS or GET_VALUE
   */
  enum class Phase {
    READ_KEY_LENGTH,
    READ_VALUE_LENGTH,
    READ_KEY,
    READ_VALUE,
    READY,
    COMPLETE
  };

  /* FastCGI key-value pairs are structured data with variable size length
   * fields. The lengths of both keys and values can be encoded as single bytes
   * or words. The high bit on the highest order byte determines the width of
   * the field. The layout is as follows
   *
   * nameLength (1 or 4 bytes)
   * valueLength (1 or 4 bytes)
   * name (nameLength many bytes)
   * value (valueLength many bytes)
   *
   * nameLength and valueLength are big-endian and the first bit is 0 to if
   * 1 byte and 1 if 4 bytes. This but must be masked out.
   */

  // Run the parser
  void process();

  // Routines for processing a key-value stream and updating the state machine
  bool parseKeyValue(folly::io::Cursor& cursor, size_t& available);
  bool parseKeyValueLength(folly::io::Cursor& cursor,
                           size_t& available,
                           size_t& lengthReturn);
  bool parseKeyValueContent(folly::io::Cursor& cursor,
                            size_t& available,
                            size_t& length,
                            folly::IOBufQueue& queue);

  Phase m_phase{Phase::READ_KEY_LENGTH}; // k/v processing state

  // Buffer for unread data
  folly::IOBufQueue m_readBuf {folly::IOBufQueue::cacheChainLength()};

  // processed fields
  size_t m_keyLength;
  size_t m_keyLeft;
  size_t m_valueLength;
  size_t m_valueLeft;

  // Buffers for partially processed data
  folly::IOBufQueue m_keyBuf   {folly::IOBufQueue::cacheChainLength()};
  folly::IOBufQueue m_valueBuf {folly::IOBufQueue::cacheChainLength()};
};
}

/*
 * FastCGISession tracks the state of an active connection with a FastCGI
 * webserver. The session is alive for as long as the connection remains open,
 * or any unfinished transports exist, and can serve a single request at a time.
 *
 * Sessions decode and encode FastCGI records to facilitate communication
 * between the webserver and the FastCGITransport (which in turn is responsible
 * for managing the VM). The session manages the network connection to the
 * webserver and the state of the FastCGI stream.
 *
 * ===== Object ownership and life cycle =====
 *
 * Life-cycle management is confusing with sessions. Rather than being "owned"
 * a session is responsible for destroying itself. The session must ensure that
 * when it is destroyed that no further data will be sent to it. In particular
 * a session cannot be destructed while either a transport exists which may want
 * to send onStdIn, onStdOut, and onComplete messages to it; pending writes may
 * have their status reported to the session via writeSuccess or writeErr; or
 * the socket is still open for reading and may deliver readDataAvailable,
 * readErr, or readEOF messages.
 *
 * To manage these different events the session tracks a master event count
 * of all anticipated and unreceived callbacks. The session also stores a flag
 * indicating that it wishes to shutdown, so that when the final anticipated
 * event occurs the event handler can call destroy(); deallocating the session.
 *
 * ===== Socket ownership and life cycle =====
 *
 * Ownership of the TCP/UNIX socket is defined by the FastCGI protocol. Upon
 * reciept of a new request via a BEGIN_REQUEST record the session will read
 * the flags field to determine whether or not it should remain open to process
 * subsequent requests following request completion.
 *
 * Protocol errors, will cause the connection to be reset to avoid deadlock or
 * mangled data transmission. Additionally a timeout will be used to ensure that
 * if the channel stops receiving data it will shutdown. If the complete request
 * has arrived the timeout will not drop the connection, we rely on the VM to
 * time itself out if execution takes too long.
 *
 * ===== Threading and synchronicity =====
 *
 * All access to internal session state should be done in the event base thread
 * as no locking or synchronization is done on internal structures. Callbacks
 * from async socket for reading and writing, and from managed connection for
 * timer and connectivity events occur in this thread.
 *
 * Calls from FastCGITransport are invoked through its own worker thread. These
 * callbacks execute their logic in the event base thread and return no data
 * to the transaction. Asynchronous calls /to/ the transport occur in the
 * session event base and are synchronized appropriately within the tranport.
 *
 * ===== Implementation notes =====
 *
 * Details about life-cycle, event counting, i/o, and transports can be found
 * detailed below. All reads should be processed through readDataAvailable,
 * and all writes sent via enqueueWrite, object destruction should be managed
 * with shutdown() or by manually checking eventCount and calling destroy.
 *
 * NEVER call delete directly.
 * NEVER call destroy without first checking the event count.
 */
struct FastCGISession
  : public  folly::wangle::ManagedConnection
  , private folly::AsyncSocket::ReadCallback
  , private folly::AsyncSocket::WriteCallback
{
  FastCGISession(
    folly::EventBase* evBase,
    JobQueueDispatcher<FastCGIWorker>& dispatcher,
    folly::AsyncSocket::UniquePtr sock,
    const folly::SocketAddress& localAddr,
    const folly::SocketAddress& peerAddr);

  //////////////////////////////////////////////////////////////////////////////
  // Callbacks
  //
  // FastCGISession has an absurd number of callbacks to handle asynchronous
  // reading and writing on its socket, and communication with the
  // FastCGITransport that runs the request.

  // ManagedConnection callbacks
  void timeoutExpired() noexcept override;
  void describe(std::ostream& os) const override;
  bool isBusy() const override;
  void notifyPendingShutdown() override;
  void closeWhenIdle() override;
  void dropConnection() override;
  void dumpConnectionState(uint8_t loglevel) override;

private:
  // Async read callbacks
  void getReadBuffer(void** bufReturn, size_t* lenReturn) override;
  void readDataAvailable(size_t len) noexcept override;
  void readEOF() noexcept override;
  void readErr(const folly::AsyncSocketException&) noexcept override;

  // Async write callbacks
  void writeErr(size_t, const folly::AsyncSocketException&) noexcept override;
  void writeSuccess() noexcept override;

public:
  // Callbacks to send data back to webserver for FastCGITransport. Ideally
  // these would also be private but the transport needs access to them.
  //
  // NB: FastCGITransport runs in its own thread and these callbacks need to
  //     explicitly place their work onto the event base thread!
  void onStdOut(std::unique_ptr<folly::IOBuf> chain);
  void onComplete();

private:
  //////////////////////////////////////////////////////////////////////////////
  // Internal State and IO
  //
  // These methods control life-cycle for FastCGISession. The session is alive
  // for as long as either the connection is open, or an unfinished transport
  // is in flight.
  //
  // If the connection is reset output from the transport is swallowed; the
  // session tracks the number of pending events internally to prevent premature
  // deallocation.

  // Common entry for readEOF, readErr, and writeErr; notifies the transport
  // that no further data is available and logs the event as received. Once
  // no further events are pending the session will destroy itself.
  void ioStop() noexcept;

  // Private to prevent accidental deletes; use destroy() so that the Destructor
  // Guards actually work
  ~FastCGISession() noexcept override {}

  // Ends the session by destroying the connection once no further events are
  // pending
  void shutdown() noexcept;

  // Send data to socket
  void enqueueWrite(std::unique_ptr<folly::IOBuf> chain);

  //////////////////////////////////////////////////////////////////////////////
  // Methods for reading records
  //
  // Convenience methods for parsing FastCGI records received from the
  // webserver. All methods are called from readDataAvailable, and guarenteed
  // to have DestructorGuard on the stack.
  //
  // onRecordImpl methods process discrete records which have already been
  // checked for length
  //
  // onStream methods process streams which may need additional buffering

  // Wrapper for checking contentLength of fixed length records
  template <typename T>
  typename std::enable_if<std::is_base_of<fcgi::record, T>::value>
  ::type onRecord(const T* rec) {
    if (rec->contentLength + sizeof(fcgi::record) < sizeof(T)) {
      // record is too short
      dropConnection();
      return;
    }

    onRecordImpl(rec);
  }

  void onRecordImpl(const fcgi::begin_record*); // FCGI_BEGIN_REQUEST
  void onRecordImpl(const fcgi::abort_record*); // FCGI_ABORT_REQUEST

  // FCGI_STDIN, FCGI_GET_VALUES, and FCGI_PARAMS
  void onStream(const fcgi::stdin_record*,  const folly::IOBuf*);
  void onStream(const fcgi::values_record*, const folly::IOBuf*);
  void onStream(const fcgi::params_record*, const folly::IOBuf*);

  //////////////////////////////////////////////////////////////////////////////
  // Methods for writing records
  //
  // Convenience methods for constructing FastCGI records to send to the
  // webserver.
  //
  // writeCapability will respond to a GET_VALUE record with the appropriate
  // capability string.
  //
  // writeEndRequest and writeUnknownType send discrete records
  //
  // writeStream will write either FCGI_STDIN or FCGI_STDOUT streams

  // Respond to the webserver with a requested capability
  void writeCapability(const std::string& key); // Send FCGI_GET_VALUES_RESULT

  // Send FCGI_END_REQUEST
  // NOTE: writeEndRequest takes a request_id as m_requestId may be different
  //       or uninitialized when using this method to decline a request
  void writeEndRequest(uint16_t request_id,
                       uint32_t app_status,
                       fcgi::Status proto_status);

  void writeUnknownType(fcgi::Type record_type); // FCGI_UNKNOWN_TYPE

  // Write stream records
  void writeStream(fcgi::Type record_type,
                   std::unique_ptr<folly::IOBuf> stream_chain);

  // Construct a new IOBuf containing a single record entry
  template<typename T>
  typename std::enable_if<
    std::is_base_of<fcgi::record, T>::value,
    std::tuple<std::unique_ptr<folly::IOBuf>, T*>
  >::type createRecord(fcgi::Type type, uint16_t rid, size_t extra = 0) {
    uint16_t len = sizeof(T) - sizeof(fcgi::record);
    auto buf = folly::IOBuf::create(sizeof(T) + extra);
    buf->append(sizeof(T));
    auto rec = reinterpret_cast<T*>(buf->writableData());
    rec->version = fcgi::Version::Current;
    rec->type = type;
    rec->requestId = rid;
    rec->contentLength = len;
    rec->paddingLength = 0;

    return std::make_tuple(std::move(buf), std::move(rec));
  }

  //////////////////////////////////////////////////////////////////////////////
  // Concurrency- these objects are owned by FastCGIServer
  //
  // These fields will out-live the connection and are part of the server state

  folly::EventBase* m_eventBase;                   // our event thread
  JobQueueDispatcher<FastCGIWorker>& m_dispatcher; // dispatching vm jobs

  //////////////////////////////////////////////////////////////////////////////
  // Connection- sockets and addresses
  //
  // These fields are details pertaining to the connection with the webserver

  folly::SocketAddress m_localAddr;     // app server address
  folly::SocketAddress m_peerAddr;      // webserver address
  folly::AsyncSocket::UniquePtr m_sock; // async socket

  //////////////////////////////////////////////////////////////////////////////
  // State data- flags, buffers, and event counters
  //
  // These fields store the state of the FastCGI record stream and track IO and
  // transport events

  folly::IOBufQueue m_readBuf; // buffer for async socket reads

  uint16_t m_requestId{0}; // the request active on the stream; 0 if no request
  bool m_keepConn{false};  // the webserver asked us to remain open after the
                           // current request completes
  bool m_shutdown{false};  // indicates that we should terminate once pending
                           // writes complete
  bool m_aborting{false};  // waiting for a transport to finish after a call
                           // to FCGI_ABORT_REQUEST

  // Key-value stream data readers
  fcgi::KVParser m_paramsReader; // state machine for FCGI_PARAMS
  fcgi::KVParser m_capReader;    // state machine for FCGI_GET_VALUES

  // Event count is the number of pending writes that have not called the
  // writeSuccess() or writeError() callbacks yet, plus one if we have not
  // yet received a readEOF(). Additionally we consider a pending call to
  // onComplete() from a currently open transport a pending event as the
  // transport may try to call us to write data until it completes.
  size_t m_eventCount{0};

  //////////////////////////////////////////////////////////////////////////////
  // Request data- transport and flags
  //
  // Transport information pertaining to a single request currently executing.
  // Once m_headersComplete is set the request is enqueued in the VM and will
  // run until onComplete() is called. the eventCount must be incremented once
  // headers are complete to reflect this.
  //
  // If thie socket shuts down before the entirety of the POST data arrives the
  // onBodyComplete callback on the transport must be invoked to notify it that
  // no further data will arrive; otherwise it will deadlock.

  // The transport is created by the session, however, once it begins its
  // execution the VM will also own a copy. Once it's made its final
  // call to onComplete, or if we terminate before enqueuing it, we can reset
  // our pointer; It cannot be freed unilaterally if we have begun execution of
  // the VM (hence the shared_ptr).
  //
  // Even though no further calls to the session will be made by the transport
  // after onComplete the runtime will continue to access it to log the request
  // and extract other post-completion data. The server/vm will release their
  // shared_ptr when such cleanup is complete.
  std::shared_ptr<FastCGITransport> m_transport;

  // Need to guard against passing data from these streams back to the
  // transport after they have been marked as complete. This is particularly
  // true of headers, if we complete them twice we may try to re-enqueue the
  // same php request in the vm.
  bool m_headersComplete{false}; // indicates execution has started
  bool m_bodyComplete{false};    // indicates no further data for the request
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_FASTCGI_FASTCGI_SESSION_H_

