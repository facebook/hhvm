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

#pragma once

#include <folly/io/IOBuf.h>
#include <folly/Range.h>

namespace HPHP {
namespace stream_transport {

/*
 * StreamTransport provides an interface for a request to read and write data
 * through a single connection for the lifetime of the request. A connection
 * can be a single HTTP request/response, a unix socket etc.
 */

struct StreamTransport {
  using OnDataType = std::function<void(std::unique_ptr<folly::IOBuf>)>;
  using OnCloseType = std::function<void()>;

  /// Delete the StreamTransport.
  ///
  /// If the connection is open then performs `closeNow()`. If `close()` was
  /// called before deleting the object then deleting the object will wait until
  /// all data is written before proceeding.
  virtual ~StreamTransport() { }

  /// Flush all pending writes and close the transport stream. This may not
  /// actually close the physical connection (in connection pooling, for
  /// example) but it signals to the remote end that the connection has been
  /// severed.
  ///
  /// It is safe to delete this object even with pending writes - it is up to
  /// StreamTransport::~StreamTransport() to wait until it is safe to continue
  /// before actually deleting the underlying data.
  ///
  /// Does not call the onClose callback when finished.
  virtual void close() = 0;

  /// Abandon pending writes and close the transport stream immediately.
  virtual void closeNow() = 0;

  /// Returns true if the connection is closed.
  ///
  /// After `closeNow()` will return true immediately.
  /// After `close()` will return true only once all bytes are flushed.
  virtual bool isClosed() const = 0;
  virtual bool isClosing() const = 0;

  virtual void write(folly::StringPiece data) = 0;

  template<typename T>
  void writeRawObject(const T& t) {
    write(folly::StringPiece{(const char*)&t, (const char*)(&t + 1)});
  }

  void writeUint8(uint8_t c) { writeRawObject(c); }
  void writeUint32(uint32_t i) { writeRawObject(i); }

  /// Sets the `onData` callback.
  ///
  /// Called when new data is available from the remote. The data
  /// may not persist after the callback returns; it's the responsibility of the
  /// callback to copy the data if required.
  ///
  /// This callback will almost certainly be called on a non-request thread.
  ///
  /// When this is called if there is pending data the callback may be
  /// immediately called before returning.
  virtual void setOnData(OnDataType callback) = 0;

  /// Sets the `onClose` callback.
  ///
  /// Called when the remote closes the connection.
  ///
  /// This callback will almost certainly be called on a non-request thread.
  virtual void setOnClose(OnCloseType callback) = 0;
};

} // namespace stream_transport
} // namespace HPHP
