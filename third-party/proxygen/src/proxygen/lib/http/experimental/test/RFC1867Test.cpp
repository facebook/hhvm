/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/experimental/RFC1867.h>

#include <proxygen/lib/http/codec/test/TestUtils.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

using namespace testing;
using folly::IOBuf;
using folly::IOBufQueue;
using std::map;
using std::pair;
using std::string;
using std::unique_ptr;

namespace {

const std::string kTestBoundary("abcdef");

/** make multipart content with optional 'filename' parameter
 * @param simpleFields fields containing only 'name' parameter and text value
 * @param explicitFiles name => {filename, file content}
 * @param randomFiles name => {filename, filesize}
 */
unique_ptr<IOBuf> makePost(
    const map<string, string>& simpleFields,
    const map<string, pair<string, string>>& explicitFiles,
    const map<string, pair<string, size_t>>& randomFiles,
    const string optExpHeaderSeqEnding = "") {
  IOBufQueue result;
  for (const auto& kv : simpleFields) {
    result.append("--");
    result.append(kTestBoundary);
    result.append("\r\nContent-Disposition: form-data; name=\"");
    result.append(kv.first);
    result.append("\"\r\n\r\n");
    result.append(kv.second);
    result.append("\r\n");
  }
  for (const auto& kv : explicitFiles) {
    result.append("--");
    result.append(kTestBoundary);
    result.append("\r\nContent-Disposition: form-data; name=\"");
    result.append(kv.first + "\"");
    auto& file = kv.second;
    if (!file.first.empty()) {
      result.append("; filename=\"" + file.first + "\"");
    }
    result.append(
        "\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n");
    result.append(IOBuf::copyBuffer(file.second.data(), file.second.length()));
    result.append("\r\n");
  }
  for (const auto& kv : randomFiles) {
    result.append("--");
    result.append(kTestBoundary);
    result.append("\r\nContent-Disposition: form-data; name=\"");
    result.append(kv.first + "\"");
    auto& file = kv.second;
    if (!file.first.empty()) {
      result.append("; filename=\"" + file.first + "\"");
    }
    result.append(
        "\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n");
    result.append(proxygen::makeBuf(file.second));
    result.append("\r\n");
  }
  result.append("--");
  result.append(kTestBoundary);
  result.append(optExpHeaderSeqEnding);

  return result.move();
}

} // namespace

namespace proxygen {

class Mock1867Callback : public RFC1867Codec::Callback {
 public:
  MOCK_METHOD(int,
              onFieldStartImpl,
              (const string& name,
               const std::string& filename,
               std::shared_ptr<HTTPMessage> msg,
               uint64_t bytesProcessed));
  int onFieldStartImpl(const string& name,
                       const std::string& filename,
                       std::unique_ptr<HTTPMessage> msg,
                       uint64_t bytesProcessed) {
    std::shared_ptr<HTTPMessage> sh_msg(msg.release());
    return onFieldStartImpl(name, filename, sh_msg, bytesProcessed);
  }
  int onFieldStart(const string& name,
                   folly::Optional<std::string> filename,
                   std::unique_ptr<HTTPMessage> msg,
                   uint64_t bytesProcessed) override {
    return onFieldStartImpl(
        name, filename.value_or(""), std::move(msg), bytesProcessed);
  }
  MOCK_METHOD(int, onFieldData, (std::shared_ptr<folly::IOBuf>, uint64_t));
  int onFieldData(std::unique_ptr<folly::IOBuf> data,
                  uint64_t bytesProcessed) override {
    std::shared_ptr<IOBuf> sh_data(data.release());
    return onFieldData(sh_data, bytesProcessed);
  }

  MOCK_METHOD(void, onFieldEnd, (bool, uint64_t));
  MOCK_METHOD(void, onError, ());
};

class RFC1867Base {
 public:
  void SetUp() {
    codec_.setCallback(&callback_);
  }

  void parse(unique_ptr<IOBuf> input, size_t chunkSize = 0) {
    IOBufQueue ibuf{IOBufQueue::cacheChainLength()};
    ibuf.append(std::move(input));
    if (chunkSize == 0) {
      chunkSize = ibuf.chainLength();
    }
    unique_ptr<IOBuf> rem;
    while (!ibuf.empty()) {
      auto chunk = ibuf.split(std::min(chunkSize, ibuf.chainLength()));
      if (rem) {
        rem->prependChain(std::move(chunk));
        chunk = std::move(rem);
      }
      rem = codec_.onIngress(std::move(chunk));
    }
    codec_.onIngressEOM();
  }

 protected:
  void testSimple(unique_ptr<IOBuf> data,
                  size_t fileSize,
                  size_t splitSize,
                  size_t parts);

  StrictMock<Mock1867Callback> callback_;
  RFC1867Codec codec_{kTestBoundary};
};

class RFC1867Test
    : public testing::Test
    , public RFC1867Base {
 public:
  void SetUp() override {
    RFC1867Base::SetUp();
  }
};

/**
 * @param data full multipart content
 * @param filesize sum of all parts
 * @param parts number of parts
 */
void RFC1867Base::testSimple(unique_ptr<IOBuf> data,
                             size_t fileSize,
                             size_t splitSize,
                             size_t parts) {
  size_t fileLength = 0;
  IOBufQueue parsedData{IOBufQueue::cacheChainLength()};
  EXPECT_CALL(callback_, onFieldStartImpl(_, _, _, _))
      .Times(parts)
      .WillRepeatedly(Return(0));
  EXPECT_CALL(callback_, onFieldData(_, _))
      .WillRepeatedly(Invoke([&](std::shared_ptr<IOBuf> data, uint64_t) {
        fileLength += data->computeChainDataLength();
        parsedData.append(data->clone());
        return 0;
      }));
  EXPECT_CALL(callback_, onFieldEnd(true, _))
      .Times(parts)
      .WillRepeatedly(Return());
  parse(data->clone(), splitSize);
  auto parsedDataBuf = parsedData.move();
  if (fileLength > 0) {
    // isChained() called from coalesce below asserts if no data has
    // been added
    parsedDataBuf->coalesce();
  }
  CHECK_EQ(fileLength, fileSize);
}

TEST_F(RFC1867Test, TestSimplePost) {
  size_t fileSize = 17;
  auto data = makePost(
      {{"foo", "bar"}, {"jojo", "binky"}}, {}, {{"file1", {"", fileSize}}});
  testSimple(std::move(data), 3 + 5 + fileSize, 0, 3);
}

TEST_F(RFC1867Test, TestSplits) {
  for (size_t i = 1; i < 500; i++) {
    size_t fileSize = 1000 + i;
    auto data = makePost(
        {{"foo", "bar"}, {"jojo", "binky"}}, {}, {{"file1", {"", fileSize}}});
    testSimple(std::move(data), 3 + 5 + fileSize, i, 3);
  }
}

TEST_F(RFC1867Test, TestSplitsWithFilename) {
  for (size_t i = 1; i < 500; i++) {
    size_t fileSize = 1000 + i;
    auto data = makePost({{"foo", "bar"}, {"jojo", "binky"}},
                         {},
                         {{"file1", {"file1.txt", fileSize}}});
    testSimple(std::move(data), 3 + 5 + fileSize, i, 3);
  }
}

TEST_F(RFC1867Test, TestHeadersChunkExtraCr) {
  // We are testing here that we correctly chunk when the parser has just
  // finished parsing a CR.
  auto numCRs = 5;
  auto headerEndingSeq = "--" + string(numCRs, '\r') + "\n";
  auto fileSize = 10;
  auto data = makePost({{"foo", "bar"}, {"jojo", "binky"}},
                       {},
                       {{"file1", {"", fileSize}}},
                       headerEndingSeq);
  // Math ensures we the parser will chunk at a '\r' with a numCRs-1
  testSimple(std::move(data), 3 + 5 + fileSize, numCRs - 1, 3);
}

class RFC1867CR
    : public testing::TestWithParam<string>
    , public RFC1867Base {
 public:
  void SetUp() override {
    RFC1867Base::SetUp();
  }
};

TEST_P(RFC1867CR, Test) {
  for (size_t i = 1; i < GetParam().size(); i++) {
    auto data = makePost({{"foo", "bar"}, {"jojo", "binky"}},
                         {{"file1", {"dummy file name", GetParam()}}},
                         {});
    testSimple(std::move(data), 3 + 5 + GetParam().size(), i, 3);
  }
}

INSTANTIATE_TEST_SUITE_P(ValueTest,
                         RFC1867CR,
                         ::testing::Values(
                             // embedded \r\n
                             string("zyx\r\nwvu", 8),
                             // leading \r
                             string("\rzyxwvut", 8),
                             // trailing \r
                             string("zyxwvut\r", 8),
                             // leading \n
                             string("\nzyxwvut", 8),
                             // trailing \n
                             string("zyxwvut\n", 8),
                             // all \r\n
                             string("\r\n\r\n\r\n\r\n", 8),
                             // all \r
                             string("\r\r\r\r\r\r\r\r", 8)));

} // namespace proxygen
