// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/crypto/aead/test/Mocks.h>
#include <fizz/record/RecordLayerUtils.h>

#include <folly/String.h>

using namespace folly;

namespace fizz {
namespace test {

class RecordLayerUtilsTest : public testing::Test {
 protected:
  static Buf
  getBuf(const std::string& hex, size_t headroom = 0, size_t tailroom = 0) {
    auto data = unhexlify(hex);
    return IOBuf::copyBuffer(data.data(), data.size(), headroom, tailroom);
  }

  static void expectSame(const Buf& buf, const std::string& hex) {
    auto str = buf->to<std::string>();
    EXPECT_EQ(hexlify(str), hex);
  }
};

TEST_F(RecordLayerUtilsTest, TestParseAndRemoveContentType) {
  // Test application_data content type
  auto buf = getBuf("abcdef17");
  auto maybeContentType = RecordLayerUtils::parseAndRemoveContentType(buf);
  EXPECT_TRUE(maybeContentType.has_value());
  EXPECT_EQ(*maybeContentType, ContentType::application_data);
  expectSame(buf, "abcdef");

  // Test handshake content type
  buf = getBuf("abcdef16");
  maybeContentType = RecordLayerUtils::parseAndRemoveContentType(buf);
  EXPECT_TRUE(maybeContentType.has_value());
  EXPECT_EQ(*maybeContentType, ContentType::handshake);
  expectSame(buf, "abcdef");

  // Test alert content type
  buf = getBuf("abcdef15");
  maybeContentType = RecordLayerUtils::parseAndRemoveContentType(buf);
  EXPECT_TRUE(maybeContentType.has_value());
  EXPECT_EQ(*maybeContentType, ContentType::alert);
  expectSame(buf, "abcdef");
}

TEST_F(RecordLayerUtilsTest, TestParseAndRemoveContentTypeWithPadding) {
  // Test with padding zeros
  auto buf = getBuf("abcdef17000000");
  auto maybeContentType = RecordLayerUtils::parseAndRemoveContentType(buf);
  EXPECT_TRUE(maybeContentType.has_value());
  EXPECT_EQ(*maybeContentType, ContentType::application_data);
  expectSame(buf, "abcdef");

  // Test with all padding and content type
  buf = getBuf("17000000");
  maybeContentType = RecordLayerUtils::parseAndRemoveContentType(buf);
  EXPECT_TRUE(maybeContentType.has_value());
  EXPECT_EQ(*maybeContentType, ContentType::application_data);
  EXPECT_TRUE(buf->empty());
}

TEST_F(RecordLayerUtilsTest, TestParseAndRemoveContentTypeNone) {
  // Test with all zeros (no content type)
  auto buf = getBuf("00000000");
  auto maybeContentType = RecordLayerUtils::parseAndRemoveContentType(buf);
  EXPECT_FALSE(maybeContentType.has_value());
  EXPECT_TRUE(buf->empty());
}

TEST_F(RecordLayerUtilsTest, TestParseAndRemoveContentTypeChained) {
  // Test with chained IOBufs
  auto buf1 = getBuf("abcdef");
  auto buf2 = getBuf("17");
  buf1->prependChain(std::move(buf2));

  auto buf = std::move(buf1);
  auto maybeContentType = RecordLayerUtils::parseAndRemoveContentType(buf);
  EXPECT_TRUE(maybeContentType.has_value());
  EXPECT_EQ(*maybeContentType, ContentType::application_data);
  expectSame(buf, "abcdef");

  // More complex chained case
  buf1 = getBuf("abcd");
  buf2 = getBuf("ef");
  auto buf3 = getBuf("16");
  auto buf4 = getBuf("000000");

  buf1->prependChain(std::move(buf2));
  buf1->prependChain(std::move(buf3));
  buf1->prependChain(std::move(buf4));

  buf = std::move(buf1);
  maybeContentType = RecordLayerUtils::parseAndRemoveContentType(buf);
  EXPECT_TRUE(maybeContentType.has_value());
  EXPECT_EQ(*maybeContentType, ContentType::handshake);
  expectSame(buf, "abcdef");
}

TEST_F(RecordLayerUtilsTest, TestWriteEncryptedRecord) {
  MockAead aead;

  auto plaintext = folly::IOBuf::create(10);
  plaintext->append(10);
  memset(plaintext->writableData(), 'X', plaintext->length());

  std::array<uint8_t, RecordLayerUtils::kEncryptedHeaderSize> headerBuf{};
  auto header = folly::IOBuf::wrapBufferAsValue(folly::range(headerBuf));

  auto ciphertext = folly::IOBuf::create(10);
  ciphertext->append(10);
  memset(ciphertext->writableData(), 'Y', ciphertext->length());

  // Set up expectation for the encrypt method - return our prepared ciphertext
  EXPECT_CALL(aead, _encrypt(_, _, 42, _))
      .WillOnce(Invoke([&ciphertext](
                           std::unique_ptr<IOBuf>&,
                           const IOBuf*,
                           uint64_t,
                           Aead::AeadOptions) { return ciphertext->clone(); }));

  auto result = RecordLayerUtils::writeEncryptedRecord(
      std::move(plaintext),
      &aead,
      &header,
      42, // seqNum
      true, // useAdditionalData
      Aead::AeadOptions());

  // Verify result is not null
  EXPECT_TRUE(result != nullptr);

  // Verify result contains some data
  EXPECT_FALSE(result->empty());
}

} // namespace test
} // namespace fizz
