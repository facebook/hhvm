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

  virtual ~StreamTransport() { }

  virtual void close() = 0;
  virtual void closeNow() = 0;
  virtual bool isClosed() const = 0;

  virtual void write(folly::StringPiece data) = 0;

  template<typename T>
  void writeRawObject(const T& t) {
    write(folly::StringPiece{(const char*)&t, (const char*)(&t + 1)});
  }

  void writeUint8(uint8_t c) { writeRawObject(c); }
  void writeUint32(uint32_t i) { writeRawObject(i); }

  virtual bool isReady() const = 0;

  // These callbacks maybe called on a non-request thread.
  virtual void doOnData(std::unique_ptr<folly::IOBuf> data) = 0;
  virtual void doOnClose() = 0;

  // Set the callback that will be called when new data is available.
  // The data may not persist after the callback returns;
  // it's the responsibility of the callback to copy the data if required.
  virtual void setOnData(OnDataType callback) = 0;
  // Set the callback that will be called when the connection is closed.
  virtual void setOnClose(OnCloseType callback) = 0;

protected:
  OnDataType onData{nullptr};
  OnCloseType onClose{nullptr};
};

} // namespace stream_transport
} // namespace HPHP
