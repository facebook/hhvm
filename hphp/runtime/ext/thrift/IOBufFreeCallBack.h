/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include <functional>
#include <memory>
#include <thread>

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP::thrift {

// Context for free function
struct IOBufFreeCallBack {
 public:
  template <typename F>
  explicit IOBufFreeCallBack(F&& releaseFunction)
      : sendBufMemoryAllocated(0),
        releaseFunction_(std::forward<F>(releaseFunction)),
        threadId_(std::this_thread::get_id()){}

  void releaseAllocatedIOBufMemory() {
    if (releaseFunction_) {
      DCHECK(threadId_ != std::this_thread::get_id());
      releaseFunction_(sendBufMemoryAllocated);
    }
  }
  uint64_t sendBufMemoryAllocated;

 private:
  std::function<void(uint64_t)> releaseFunction_;
  std::thread::id threadId_;
};

std::unique_ptr<folly::IOBuf> createIOBufWithMemoryTracking(
    const String& response,
    std::function<void(uint64_t)> cbToRecordAllocFreeInIOThread) {
  IOBufFreeCallBack* cbPtr = new IOBufFreeCallBack(cbToRecordAllocFreeInIOThread);

  auto acquiredAllocFree = [](void* buf, void* userData) {
    IOBufFreeCallBack* cb = static_cast<IOBufFreeCallBack*>(userData);
    if (cb) {
      cb->releaseAllocatedIOBufMemory();
      delete cb;
    }
    free(buf);
  };

  MemoryManager::CountMalloc counter(*tl_heap, cbPtr->sendBufMemoryAllocated);
  // Allocate buffer and copy data
  void* buf = malloc(response.size());
  if (!buf) throw std::bad_alloc();
  memcpy(buf, response.data(), response.size());
  // Create IOBuf with custom free function
  return folly::IOBuf::takeOwnership(
      buf, response.size(), acquiredAllocFree, cbPtr);
}
} // namespace HPHP::thrift
