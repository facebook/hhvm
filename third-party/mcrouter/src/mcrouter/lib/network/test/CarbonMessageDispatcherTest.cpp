/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstring>

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>

#include "mcrouter/lib/network/CarbonMessageDispatcher.h"
#include "mcrouter/lib/network/McServerRequestContext.h"

#include "mcrouter/lib/network/gen/MemcacheMessages.h"

using namespace facebook::memcache;

using CarbonTestMessageList = List<McGetRequest, McSetRequest>;

// using facebook::memcache::CarbonMessageDispatcher; will break the GCC build
// until https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59815 is fixed
struct TestCallback
    : public CarbonMessageDispatcher<CarbonTestMessageList, TestCallback> {
  std::function<void(McGetRequest&&)> onGet_;
  std::function<void(McSetRequest&&)> onSet_;

  template <class F, class B>
  TestCallback(F&& onGet, B&& onSet)
      : onGet_(std::move(onGet)), onSet_(std::move(onSet)) {}

  void onTypedMessage(
      const CaretMessageInfo& /* headerInfo */,
      const folly::IOBuf& /* reqBuffer */,
      McGetRequest&& req) {
    onGet_(std::move(req));
  }

  void onTypedMessage(
      const CaretMessageInfo& /* headerInfo */,
      const folly::IOBuf& /* reqBuffer */,
      McSetRequest&& req) {
    onSet_(std::move(req));
  }
};

TEST(CarbonMessage, basic) {
  /* construct a request */
  McGetRequest get;
  get.key_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "12345");

  /* serialize into an iobuf */
  carbon::CarbonQueueAppenderStorage storage;
  carbon::CarbonProtocolWriter writer(storage);
  get.serialize(writer);

  folly::IOBuf body(folly::IOBuf::CREATE, storage.computeBodySize());
  const auto iovs = storage.getIovecs();
  for (size_t i = 0; i < iovs.second; ++i) {
    const struct iovec* iov = iovs.first + i;
    std::memcpy(body.writableTail(), iov->iov_base, iov->iov_len);
    body.append(iov->iov_len);
  }

  CaretMessageInfo headerInfo1;
  CaretMessageInfo headerInfo2;
  headerInfo1.typeId = 1;
  headerInfo2.typeId = 2;
  headerInfo1.bodySize = storage.computeBodySize();
  headerInfo2.bodySize = headerInfo1.bodySize;

  folly::IOBuf requestBuf1(folly::IOBuf::CREATE, 1024);
  folly::IOBuf requestBuf2(folly::IOBuf::CREATE, 1024);

  headerInfo1.headerSize = caretPrepareHeader(
      headerInfo1, reinterpret_cast<char*>(requestBuf1.writableTail()));
  requestBuf1.append(headerInfo1.headerSize);
  headerInfo2.headerSize = caretPrepareHeader(
      headerInfo2, reinterpret_cast<char*>(requestBuf2.writableTail()));
  requestBuf2.append(headerInfo2.headerSize);

  requestBuf1.appendChain(body.clone());
  requestBuf2.appendChain(body.clone());

  bool getCalled = false;
  bool setCalled = false;
  TestCallback cb(
      [&getCalled](McGetRequest&& req) {
        /* check unserialized request is the same as sent */
        getCalled = true;
        EXPECT_EQ(req.key_ref()->fullKey(), folly::StringPiece("12345"));
      },
      [&setCalled](McSetRequest&&) { setCalled = true; });

  bool ret;

  CaretMessageInfo info;

  /* simulate receiving the iobuf over network with some type id */
  ret = cb.dispatchTypedRequest(headerInfo2, requestBuf2);
  /* there's no type id 2, expect false */
  EXPECT_FALSE(ret);
  EXPECT_FALSE(getCalled);
  EXPECT_FALSE(setCalled);

  ret = cb.dispatchTypedRequest(headerInfo1, requestBuf1);
  EXPECT_TRUE(ret);
  EXPECT_TRUE(getCalled);
}
