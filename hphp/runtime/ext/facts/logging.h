/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <memory>
#include <string>

#include <folly/Executor.h>
#include <folly/Range.h>
#include <folly/Synchronized.h>
#include <folly/Unit.h>
#include <folly/futures/Future.h>
#include <folly/logging/LogWriter.h>

namespace HPHP {
namespace Facts {

void enableFactsLogging(
    const std::string& owner,
    const std::string& options,
    bool allow_propagation);

/*
 * AsyncLogWriter is a wrapper for other log writers which makes them
 * asynchronous by reducing the critical path of logging to a mutex lock and
 * queue insertion.  A separate thread will consume the queue and call the
 * underlying log writer's writeMessage method.
 *
 * Of note is how flush works.  Rather than waiting for all data in the queue
 * to be written, it waits for all data enqueued prior to the flush to be
 * written.
 */
struct AsyncLogWriter : public folly::LogWriter {
public:
  explicit AsyncLogWriter(std::unique_ptr<folly::LogWriter> writer);
  ~AsyncLogWriter() override {
    flush();
  }

  void writeMessage(folly::StringPiece buffer, uint32_t flags = 0) override;
  void writeMessage(std::string&& buffer, uint32_t flags = 0) override;
  void flush() override;
  bool ttyOutput() const override {
    return m_is_tty;
  }

private:
  const bool m_is_tty;
  std::unique_ptr<folly::LogWriter> m_writer;
  std::unique_ptr<folly::Executor> m_exec;
  folly::Synchronized<folly::Future<folly::Unit>> m_syncedFuture;
};

} // namespace Facts
} // namespace HPHP
