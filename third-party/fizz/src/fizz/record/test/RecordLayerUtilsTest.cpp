// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/crypto/aead/test/Mocks.h>
#include <fizz/record/BufAndPaddingPolicy.h>
#include <fizz/record/RecordLayerUtils.h>

#include <folly/String.h>

using namespace folly;

namespace fizz {
namespace test {

class MockBufAndPaddingPolicy : public BufAndPaddingPolicy {
 public:
  MOCK_CONST_METHOD2(
      getBufAndPaddingToEncrypt,
      std::pair<Buf, uint16_t>(folly::IOBufQueue&, uint16_t));
};

class RecordLayerUtilsTest : public testing::Test {
 protected:
  IOBufQueue queue_{IOBufQueue::cacheChainLength()};

  static Buf
  getBuf(const std::string& hex, size_t headroom = 0, size_t tailroom = 0) {
    auto data = unhexlify(hex);
    return IOBuf::copyBuffer(data.data(), data.size(), headroom, tailroom);
  }

  void addToQueue(const std::string& hex) {
    queue_.append(getBuf(hex));
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

TEST_F(RecordLayerUtilsTest, TestParseEncryptedRecord) {
  // Test 1: Basic successful parsing of application data
  {
    queue_.reset();

    auto header = IOBuf::create(5);
    header->append(5);
    auto headerData = header->writableData();
    headerData[0] = 0x17;
    headerData[1] = 0x03;
    headerData[2] = 0x03;
    headerData[3] = 0x00;
    headerData[4] = 0x0A;

    // Create payload: 10 'A's
    auto payload = IOBuf::create(10);
    payload->append(10);
    memset(payload->writableData(), 'A', 10);

    header->prependChain(std::move(payload));
    queue_.append(std::move(header));

    auto result = RecordLayerUtils::parseEncryptedRecord(queue_);
    EXPECT_EQ(result.contentType, ContentType::application_data);
    EXPECT_FALSE(result.continueReading);
    expectSame(result.ciphertext, "41414141414141414141");
    expectSame(result.header, "170303000a");
  }

  // Test 2: Not enough data for header
  {
    queue_.reset();
    auto partialHeader = IOBuf::create(2);
    partialHeader->append(2);
    auto headerData = partialHeader->writableData();
    headerData[0] = 0x17;
    headerData[1] = 0x03;
    queue_.append(std::move(partialHeader));

#ifdef NDEBUG
    GTEST_SKIP() << "DCHECK tests only run in debug builds";
#else
    EXPECT_DEATH(
        RecordLayerUtils::parseEncryptedRecord(queue_),
        "parseEncryptedRecord called with insufficient buffer data");
#endif
  }

  // Test 3: Not enough data for full record
  {
    queue_.reset();

    auto header = IOBuf::create(5);
    header->append(5);
    auto headerData = header->writableData();
    headerData[0] = 0x17;
    headerData[1] = 0x03;
    headerData[2] = 0x03;
    headerData[3] = 0x00;
    headerData[4] = 0x0A;

    // Create partial payload: only 2 'A's instead of 10
    auto partialPayload = IOBuf::create(2);
    partialPayload->append(2);
    memset(partialPayload->writableData(), 'A', 2);

    header->prependChain(std::move(partialPayload));
    queue_.append(std::move(header));

#ifdef NDEBUG
    GTEST_SKIP() << "DCHECK tests only run in debug builds";
#else
    EXPECT_DEATH(
        RecordLayerUtils::parseEncryptedRecord(queue_),
        "parseEncryptedRecord called with incomplete record data");
#endif
  }
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

TEST_F(RecordLayerUtilsTest, TestParseEncryptedRecordMultiple) {
  // Test with multiple records in the queue
  queue_.reset();

  // Create first record: application_data with 5 'A's
  auto header1 = IOBuf::create(5);
  header1->append(5);
  auto headerData1 = header1->writableData();
  headerData1[0] = 0x17; // application_data
  headerData1[1] = 0x03;
  headerData1[2] = 0x03;
  headerData1[3] = 0x00;
  headerData1[4] = 0x05; // length 5

  auto payload1 = IOBuf::create(5);
  payload1->append(5);
  memset(payload1->writableData(), 'A', 5);

  // Create second record: handshake with 8 'B's
  auto header2 = IOBuf::create(5);
  header2->append(5);
  auto headerData2 = header2->writableData();
  headerData2[0] = 0x16; // handshake
  headerData2[1] = 0x03;
  headerData2[2] = 0x03;
  headerData2[3] = 0x00;
  headerData2[4] = 0x08; // length 8

  auto payload2 = IOBuf::create(8);
  payload2->append(8);
  memset(payload2->writableData(), 'B', 8);

  // Create third record: change_cipher_spec (should set continueReading flag)
  auto header3 = IOBuf::create(5);
  header3->append(5);
  auto headerData3 = header3->writableData();
  headerData3[0] = 0x14; // change_cipher_spec
  headerData3[1] = 0x03;
  headerData3[2] = 0x03;
  headerData3[3] = 0x00;
  headerData3[4] = 0x01; // length 1

  auto payload3 = IOBuf::create(1);
  payload3->append(1);
  payload3->writableData()[0] = 0x01; // valid CCS value

  // Create fourth record: alert with 3 'C's
  auto header4 = IOBuf::create(5);
  header4->append(5);
  auto headerData4 = header4->writableData();
  headerData4[0] = 0x15; // alert
  headerData4[1] = 0x03;
  headerData4[2] = 0x03;
  headerData4[3] = 0x00;
  headerData4[4] = 0x03; // length 3

  auto payload4 = IOBuf::create(3);
  payload4->append(3);
  memset(payload4->writableData(), 'C', 3);

  // Add each record to the queue separately
  // First record: application_data with 5 'A's
  header1->prependChain(std::move(payload1));
  queue_.append(std::move(header1));

  // Second record: handshake with 8 'B's
  header2->prependChain(std::move(payload2));
  queue_.append(std::move(header2));

  // Third record: change_cipher_spec with 1 byte
  header3->prependChain(std::move(payload3));
  queue_.append(std::move(header3));

  // Fourth record: alert with 3 'C's
  header4->prependChain(std::move(payload4));
  queue_.append(std::move(header4));

  // Parse first record (application_data)
  auto result1 = RecordLayerUtils::parseEncryptedRecord(queue_);
  EXPECT_EQ(result1.contentType, ContentType::application_data);
  EXPECT_FALSE(result1.continueReading);
  expectSame(result1.ciphertext, "4141414141"); // 5 'A's
  expectSame(result1.header, "1703030005");

  // Parse second record (handshake)
  auto result2 = RecordLayerUtils::parseEncryptedRecord(queue_);
  EXPECT_EQ(result2.contentType, ContentType::handshake);
  EXPECT_FALSE(result2.continueReading);
  expectSame(result2.ciphertext, "4242424242424242"); // 8 'B's
  expectSame(result2.header, "1603030008");

  auto result3 = RecordLayerUtils::parseEncryptedRecord(queue_);
  EXPECT_EQ(result3.contentType, ContentType::change_cipher_spec);
  EXPECT_TRUE(result3.continueReading); // This should be true for CCS
  expectSame(result3.ciphertext, "01"); // CCS value
  expectSame(result3.header, "1403030001");

  auto result4 = RecordLayerUtils::parseEncryptedRecord(queue_);
  EXPECT_EQ(result4.contentType, ContentType::alert);
  EXPECT_FALSE(result4.continueReading);
  expectSame(result4.ciphertext, "434343"); // 3 'C's
  expectSame(result4.header, "1503030003");

  EXPECT_TRUE(queue_.empty());
}

TEST_F(RecordLayerUtilsTest, TestParseEncryptedRecordPartialMultiple) {
  // Test 1: Incomplete header
  {
    queue_.reset();
    auto partialHeader = IOBuf::create(3);
    partialHeader->append(3);
    auto headerData = partialHeader->writableData();
    headerData[0] = 0x17; // application_data
    headerData[1] = 0x03;
    headerData[2] = 0x03;
    queue_.append(std::move(partialHeader));

#ifdef NDEBUG
    GTEST_SKIP() << "DCHECK tests only run in debug builds";
#else
    EXPECT_DEATH(
        RecordLayerUtils::parseEncryptedRecord(queue_),
        "parseEncryptedRecord called with insufficient buffer data");
#endif
  }

  // Test 2: Missing payload
  {
    queue_.reset();
    auto header = IOBuf::create(5);
    header->append(5);
    auto headerData = header->writableData();
    headerData[0] = 0x17; // application_data
    headerData[1] = 0x03;
    headerData[2] = 0x03;
    headerData[3] = 0x00;
    headerData[4] = 0x06; // length 6
    queue_.append(std::move(header));

#ifdef NDEBUG
    GTEST_SKIP() << "DCHECK tests only run in debug builds";
#else
    EXPECT_DEATH(
        RecordLayerUtils::parseEncryptedRecord(queue_),
        "parseEncryptedRecord called with incomplete record data");
#endif
  }

  // Test 3: Incomplete payload
  {
    queue_.reset();
    auto header = IOBuf::create(5);
    header->append(5);
    auto headerData = header->writableData();
    headerData[0] = 0x17; // application_data
    headerData[1] = 0x03;
    headerData[2] = 0x03;
    headerData[3] = 0x00;
    headerData[4] = 0x06; // length 6

    auto partialPayload = IOBuf::create(3);
    partialPayload->append(3);
    memset(partialPayload->writableData(), 'D', 3);

    header->prependChain(std::move(partialPayload));
    queue_.append(std::move(header));

#ifdef NDEBUG
    GTEST_SKIP() << "DCHECK tests only run in debug builds";
#else
    EXPECT_DEATH(
        RecordLayerUtils::parseEncryptedRecord(queue_),
        "parseEncryptedRecord called with incomplete record data");
#endif
  }

  // Test 4: Successful parsing with partial records being added incrementally
  {
    queue_.reset();

    // First, add just a partial header
    auto partialHeader = IOBuf::create(3);
    partialHeader->append(3);
    auto headerData = partialHeader->writableData();
    headerData[0] = 0x17; // application_data
    headerData[1] = 0x03;
    headerData[2] = 0x03;
    queue_.append(std::move(partialHeader));

    // Add the rest of the header
    auto headerRest = IOBuf::create(2);
    headerRest->append(2);
    auto headerRestData = headerRest->writableData();
    headerRestData[0] = 0x00;
    headerRestData[1] = 0x06; // length 6
    queue_.append(std::move(headerRest));

    // Add partial payload
    auto partialPayload = IOBuf::create(3);
    partialPayload->append(3);
    memset(partialPayload->writableData(), 'D', 3);
    queue_.append(std::move(partialPayload));

    // Add rest of payload and start of next record
    auto restPayloadAndNextHeader = IOBuf::create(8);
    restPayloadAndNextHeader->append(8);
    auto mixedData = restPayloadAndNextHeader->writableData();
    // Rest of first record payload (3 more 'D's)
    memset(mixedData, 'D', 3);
    // Start of next record header
    mixedData[3] = 0x16; // handshake
    mixedData[4] = 0x03;
    mixedData[5] = 0x03;
    mixedData[6] = 0x00;
    mixedData[7] = 0x04; // length 4
    queue_.append(std::move(restPayloadAndNextHeader));

    // Now parse first record - should succeed
    auto result4 = RecordLayerUtils::parseEncryptedRecord(queue_);
    EXPECT_EQ(result4.contentType, ContentType::application_data);
    EXPECT_FALSE(result4.continueReading);
    expectSame(result4.ciphertext, "444444444444"); // 6 'D's
    expectSame(result4.header, "1703030006");

    // Add payload for second record
    auto secondPayload = IOBuf::create(4);
    secondPayload->append(4);
    memset(secondPayload->writableData(), 'E', 4);
    queue_.append(std::move(secondPayload));

    // Parse second record
    auto result5 = RecordLayerUtils::parseEncryptedRecord(queue_);
    EXPECT_EQ(result5.contentType, ContentType::handshake);
    EXPECT_FALSE(result5.continueReading);
    expectSame(result5.ciphertext, "45454545"); // 4 'E's
    expectSame(result5.header, "1603030004");

    EXPECT_TRUE(queue_.empty());
  }
}

TEST_F(RecordLayerUtilsTest, TestPrepareBufferWithPaddingEmptyBuffer) {
  MockAead aead;
  folly::IOBufQueue emptyQueue{folly::IOBufQueue::cacheChainLength()};
  testing::NiceMock<MockBufAndPaddingPolicy> paddingPolicy;

  // Add a small buffer to the queue to satisfy the DCHECK
  auto smallBuf = folly::IOBuf::copyBuffer("x");
  emptyQueue.append(std::move(smallBuf));

  // Set up the mock to return an empty buffer (not null) from
  // getBufAndPaddingToEncrypt
  EXPECT_CALL(paddingPolicy, getBufAndPaddingToEncrypt(_, _))
      .WillOnce(Invoke([](folly::IOBufQueue& q, uint16_t) {
        q.move();
        return std::make_pair(folly::IOBuf::create(0), uint16_t(0));
      }));

  // Test that the function handles an empty buffer correctly
  auto result = prepareBufferWithPadding(
      emptyQueue,
      ContentType::application_data,
      paddingPolicy,
      1000, // maxRecord
      &aead);

  EXPECT_TRUE(result != nullptr);
  EXPECT_FALSE(
      result->empty()); // Should contain at least the content type byte
  EXPECT_TRUE(emptyQueue.empty());
}

TEST_F(RecordLayerUtilsTest, TestPrepareBufferWithPaddingSingleRecord) {
  MockAead aead;
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};

  auto data = folly::IOBuf::copyBuffer("hello world");
  queue.append(std::move(data));

  EXPECT_CALL(aead, getCipherOverhead()).WillRepeatedly(Return(16));

  BufAndConstPaddingPolicy paddingPolicy(0);
  auto result = prepareBufferWithPadding(
      queue,
      ContentType::application_data,
      paddingPolicy,
      1000, // maxRecord
      &aead);

  EXPECT_TRUE(result != nullptr);
  EXPECT_FALSE(result->empty());

  auto copy = result->clone();
  copy->coalesce();
  EXPECT_EQ(
      copy->data()[copy->length() - 1],
      static_cast<uint8_t>(ContentType::application_data));

  EXPECT_TRUE(queue.empty());
}

TEST_F(RecordLayerUtilsTest, TestPrepareBufferWithPaddingMultipleRecords) {
  MockAead aead;
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};

  auto data = folly::IOBuf::create(1000);
  data->append(1000);
  memset(data->writableData(), 'X', data->length());
  queue.append(std::move(data));

  const size_t maxRecord = 300;

  EXPECT_CALL(aead, getCipherOverhead()).WillRepeatedly(Return(16));

  BufAndConstPaddingPolicy paddingPolicy(0);
  auto result = prepareBufferWithPadding(
      queue, ContentType::application_data, paddingPolicy, maxRecord, &aead);

  EXPECT_TRUE(result != nullptr);
  EXPECT_FALSE(result->empty());

  EXPECT_LE(result->computeChainDataLength(), maxRecord + 1);

  EXPECT_FALSE(queue.empty());

  auto result2 = prepareBufferWithPadding(
      queue, ContentType::application_data, paddingPolicy, maxRecord, &aead);

  EXPECT_TRUE(result2 != nullptr);
  EXPECT_FALSE(result2->empty());
}

TEST_F(RecordLayerUtilsTest, TestPrepareBufferWithPaddingWithPadding) {
  MockAead aead;
  testing::NiceMock<MockBufAndPaddingPolicy> paddingPolicy;
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};

  auto data = folly::IOBuf::copyBuffer("hello world");
  queue.append(std::move(data));

  EXPECT_CALL(paddingPolicy, getBufAndPaddingToEncrypt(_, _))
      .WillOnce(Invoke([](folly::IOBufQueue& q, uint16_t) {
        auto buf = q.move();
        return std::make_pair(std::move(buf), uint16_t(10));
      }));

  EXPECT_CALL(aead, getCipherOverhead()).WillRepeatedly(Return(16));

  auto result = prepareBufferWithPadding(
      queue,
      ContentType::application_data,
      paddingPolicy,
      1000, // maxRecord
      &aead);

  EXPECT_TRUE(result != nullptr);
  EXPECT_FALSE(result->empty());

  auto copy = result->clone();
  copy->coalesce();
  // Buffer should now be: "hello world" + content_type + 10 zeros
  // So length should be 11 (hello world) + 1 (content type) + 10 (padding) = 22
  EXPECT_EQ(copy->length(), 22);

  EXPECT_TRUE(queue.empty());
}

} // namespace test
} // namespace fizz
