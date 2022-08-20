/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/crypto/aead/test/Mocks.h>
#include <fizz/record/BufAndPaddingPolicy.h>
#include <fizz/record/EncryptedRecordLayer.h>
#include <folly/String.h>

using namespace folly;

namespace fizz {
namespace test {

constexpr size_t kEncryptedHeaderSize = 1 + 2 + 2;
const uint16_t paddingModulo = 50;

class EncryptedRecordTest : public testing::Test {
  void SetUp() override {
    auto readAead = std::make_unique<MockAead>();
    readAead_ = readAead.get();
    read_.setAead(folly::ByteRange(), std::move(readAead));
    auto writeAead = std::make_unique<MockAead>();
    writeAead_ = writeAead.get();
    write_.setAead(folly::ByteRange(), std::move(writeAead));
  }

 protected:
  EncryptedReadRecordLayer read_{EncryptionLevel::AppTraffic};
  EncryptedWriteRecordLayer write_{EncryptionLevel::AppTraffic};
  MockAead* readAead_;
  MockAead* writeAead_;

  IOBufQueue queue_{IOBufQueue::cacheChainLength()};

  IOBufEqualTo eq_;

  static Buf
  getBuf(const std::string& hex, size_t headroom = 0, size_t tailroom = 0) {
    auto data = unhexlify(hex);
    return IOBuf::copyBuffer(data.data(), data.size(), headroom, tailroom);
  }

  void addToQueue(const std::string& hex) {
    queue_.append(getBuf(hex));
  }

  static void expectSame(const Buf& buf, const std::string& hex) {
    auto str = buf->moveToFbString().toStdString();
    EXPECT_EQ(hexlify(str), hex);
  }
};

TEST_F(EncryptedRecordTest, TestReadEmpty) {
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_FALSE(msg.has_value());
  EXPECT_EQ(kEncryptedHeaderSize, msg.sizeHint);
}

TEST_F(EncryptedRecordTest, TestReadHandshake) {
  addToQueue("17030100050123456789");
  EXPECT_CALL(*readAead_, _decrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "0123456789");
        return getBuf("abcdef16");
      }));
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_EQ(msg->type, ContentType::handshake);
  expectSame(msg->fragment, "abcdef");
  EXPECT_TRUE(queue_.empty());
  EXPECT_EQ(msg.sizeHint, 0);
}

TEST_F(EncryptedRecordTest, TestReadAlert) {
  addToQueue("17030100050123456789");
  EXPECT_CALL(*readAead_, _decrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "0123456789");
        return getBuf("020215");
      }));
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_EQ(msg->type, ContentType::alert);
  expectSame(msg->fragment, "0202");
  EXPECT_TRUE(queue_.empty());
  EXPECT_EQ(msg.sizeHint, 0);
}

TEST_F(EncryptedRecordTest, TestReadAppData) {
  addToQueue("17030100050123456789");
  EXPECT_CALL(*readAead_, _decrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        EXPECT_FALSE(buf->isShared());
        expectSame(buf, "0123456789");
        return getBuf("1234abcd17");
      }));
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_EQ(msg->type, ContentType::application_data);
  expectSame(msg->fragment, "1234abcd");
  EXPECT_TRUE(queue_.empty());
  EXPECT_EQ(msg.sizeHint, 0);
}

TEST_F(EncryptedRecordTest, TestReadUnknown) {
  addToQueue("17030100050123456789");
  EXPECT_CALL(*readAead_, _decrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "0123456789");
        return getBuf("1234abcd20");
      }));
  EXPECT_ANY_THROW(read_.read(queue_, Aead::AeadOptions()));
}

TEST_F(EncryptedRecordTest, TestWaitForData) {
  addToQueue("1703010010012345");
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_FALSE(msg.has_value());
  EXPECT_EQ(queue_.chainLength(), 8);
  EXPECT_EQ(13, msg.sizeHint);
}

TEST_F(EncryptedRecordTest, TestWaitForHeader) {
  addToQueue("16030102");
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_FALSE(msg.has_value());
  EXPECT_EQ(queue_.chainLength(), 4);
  EXPECT_EQ(1, msg.sizeHint);
}

TEST_F(EncryptedRecordTest, TestMaxSize) {
  addToQueue("1603014100");
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_FALSE(msg.has_value());
  EXPECT_EQ(queue_.chainLength(), 5);
  EXPECT_EQ(0x4100, msg.sizeHint);
}

TEST_F(EncryptedRecordTest, TestOverSize) {
  addToQueue("1603015000");
  EXPECT_ANY_THROW(read_.read(queue_, Aead::AeadOptions()));
}

TEST_F(EncryptedRecordTest, TestDataRemaining) {
  addToQueue("17030100050123456789aa");
  EXPECT_CALL(*readAead_, _decrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "0123456789");
        return getBuf("abcdef16");
      }));
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_EQ(msg.sizeHint, 0);
  EXPECT_EQ(queue_.chainLength(), 1);
}

TEST_F(EncryptedRecordTest, TestPadding) {
  addToQueue("17030100050123456789");
  EXPECT_CALL(*readAead_, _decrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "0123456789");
        return getBuf("1234abcd17000000");
      }));
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_EQ(msg.sizeHint, 0);
  EXPECT_EQ(msg->type, ContentType::application_data);
  expectSame(msg->fragment, "1234abcd");
  EXPECT_TRUE(queue_.empty());
}

TEST_F(EncryptedRecordTest, TestAllPaddingAppData) {
  addToQueue("17030100050123456789");
  EXPECT_CALL(*readAead_, _decrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "0123456789");
        return getBuf("17000000");
      }));
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_EQ(msg->type, ContentType::application_data);
  EXPECT_TRUE(msg->fragment->empty());
  EXPECT_TRUE(queue_.empty());
}

TEST_F(EncryptedRecordTest, TestAllPaddingHandshake) {
  addToQueue("17030100050123456789");
  EXPECT_CALL(*readAead_, _decrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "0123456789");
        return getBuf("16000000");
      }));
  EXPECT_ANY_THROW(read_.read(queue_, Aead::AeadOptions()));
}

TEST_F(EncryptedRecordTest, TestNoContentType) {
  addToQueue("17030100050123456789");
  EXPECT_CALL(*readAead_, _decrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "0123456789");
        return getBuf("00000000");
      }));
  EXPECT_ANY_THROW(read_.read(queue_, Aead::AeadOptions()));
}

TEST_F(EncryptedRecordTest, TestReadSeqNum) {
  for (int i = 0; i < 10; i++) {
    addToQueue("17030100050123456789");
    EXPECT_CALL(*readAead_, _decrypt(_, _, i, _))
        .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                            const IOBuf*,
                            uint64_t,
                            Aead::AeadOptions) {
          expectSame(buf, "0123456789");
          return getBuf("1234abcd17");
        }));
    auto msg = read_.read(queue_, Aead::AeadOptions());
    EXPECT_EQ(msg.sizeHint, 0);
  }
}

TEST_F(EncryptedRecordTest, TestSkipAndWait) {
  read_.setSkipFailedDecryption(true);
  addToQueue("17030100050123456789");
  EXPECT_CALL(*readAead_, _tryDecrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& /*buf*/,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) { return folly::none; }));
  auto msg = read_.read(queue_, Aead::AeadOptions());

  // We skipped the failed record; it is effectively treated as a 0 byte read,
  // so we still need to read the header.
  EXPECT_EQ(msg.sizeHint, kEncryptedHeaderSize);

  EXPECT_FALSE(msg.has_value());
  EXPECT_TRUE(queue_.empty());
}

TEST_F(EncryptedRecordTest, TestSkipAndRead) {
  Sequence s;
  read_.setSkipFailedDecryption(true);
  addToQueue("1703010005012345678917030100050123456789170301000501234567aa");
  EXPECT_CALL(*readAead_, _tryDecrypt(_, _, 0, _))
      .InSequence(s)
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& /*buf*/,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) { return folly::none; }));
  EXPECT_CALL(*readAead_, _tryDecrypt(_, _, 0, _))
      .InSequence(s)
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "0123456789");
        return getBuf("1234abcd17");
      }));
  auto msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_EQ(msg.sizeHint, 0);
  EXPECT_EQ(msg->type, ContentType::application_data);
  expectSame(msg->fragment, "1234abcd");
  EXPECT_EQ(queue_.chainLength(), 10);
  EXPECT_CALL(*readAead_, _decrypt(_, _, 1, _))
      .InSequence(s)
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "01234567aa");
        return getBuf("1234abaa17");
      }));
  msg = read_.read(queue_, Aead::AeadOptions());
  EXPECT_EQ(msg.sizeHint, 0);
  EXPECT_EQ(msg->type, ContentType::application_data);
  expectSame(msg->fragment, "1234abaa");
  EXPECT_TRUE(queue_.empty());
}

TEST_F(EncryptedRecordTest, TestWriteHandshake) {
  TLSMessage msg{ContentType::handshake, getBuf("1234567890")};
  EXPECT_CALL(*writeAead_, _encrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "123456789016");
        return getBuf("abcd1234abcd");
      }));
  auto buf = write_.write(std::move(msg), Aead::AeadOptions());
  expectSame(buf.data, "1703030006abcd1234abcd");
}

TEST_F(EncryptedRecordTest, TestWriteAppData) {
  TLSMessage msg{ContentType::application_data, getBuf("1234567890")};
  EXPECT_CALL(*writeAead_, _encrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        expectSame(buf, "123456789017");
        return getBuf("abcd1234abcd");
      }));
  auto buf = write_.write(std::move(msg), Aead::AeadOptions());
  expectSame(buf.data, "1703030006abcd1234abcd");
}

TEST_F(EncryptedRecordTest, TestWriteAppDataInPlace) {
  TLSMessage msg{ContentType::application_data, getBuf("1234567890", 5, 17)};
  EXPECT_CALL(*writeAead_, _encrypt(_, _, 0, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        // footer should have been written w/o chaining
        EXPECT_FALSE(buf->isChained());
        expectSame(buf, "123456789017");
        // we need to return room for the header
        return getBuf("abcd1234abcd", 5, 0);
      }));
  auto buf = write_.write(std::move(msg), Aead::AeadOptions());
  EXPECT_FALSE(buf.data->isChained());
  expectSame(buf.data, "1703030006abcd1234abcd");
}

TEST_F(EncryptedRecordTest, TestFragmentedWrite) {
  TLSMessage msg{ContentType::application_data, IOBuf::create(0x4a00)};
  msg.fragment->append(0x4a00);
  memset(msg.fragment->writableData(), 0x1, msg.fragment->length());

  Sequence s;
  EXPECT_CALL(*writeAead_, _encrypt(_, _, 0, _))
      .InSequence(s)
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& /*buf*/,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) { return getBuf("aaaa"); }));
  EXPECT_CALL(*writeAead_, _encrypt(_, _, 1, _))
      .InSequence(s)
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& /*buf*/,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) { return getBuf("bbbb"); }));
  auto outBuf = write_.write(std::move(msg), Aead::AeadOptions());
  expectSame(outBuf.data, "1703034001aaaa1703030a01bbbb");
}

TEST_F(EncryptedRecordTest, TestWriteSplittingWholeBuf) {
  TLSMessage msg{ContentType::application_data, IOBuf::create(2000)};
  msg.fragment->append(2000);
  memset(msg.fragment->writableData(), 0x1, msg.fragment->length());
  msg.fragment->prependChain(IOBuf::copyBuffer("moredata"));

  Sequence s;
  EXPECT_CALL(*writeAead_, _encrypt(_, _, _, _))
      .Times(2)
      .WillRepeatedly(Invoke([](std::unique_ptr<IOBuf>& /*buf*/,
                                const IOBuf*,
                                uint64_t,
                                Aead::AeadOptions) { return getBuf("aaaa"); }));
  write_.write(std::move(msg), Aead::AeadOptions());
}

TEST_F(EncryptedRecordTest, TestWriteSplittingCombineSmall) {
  TLSMessage msg{ContentType::application_data, IOBuf::create(500)};
  msg.fragment->append(500);
  memset(msg.fragment->writableData(), 0x1, msg.fragment->length());
  msg.fragment->prependChain(IOBuf::copyBuffer("moredata"));

  Sequence s;
  EXPECT_CALL(*writeAead_, _encrypt(_, _, _, _))
      .Times(1)
      .WillRepeatedly(Invoke([](std::unique_ptr<IOBuf>& /*buf*/,
                                const IOBuf*,
                                uint64_t,
                                Aead::AeadOptions) { return getBuf("aaaa"); }));
  write_.write(std::move(msg), Aead::AeadOptions());
}

TEST_F(EncryptedRecordTest, TestWriteSeqNum) {
  for (int i = 0; i < 10; i++) {
    TLSMessage msg{ContentType::application_data, getBuf("1234567890")};
    EXPECT_CALL(*writeAead_, _encrypt(_, _, i, _))
        .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                            const IOBuf*,
                            uint64_t,
                            Aead::AeadOptions) {
          expectSame(buf, "123456789017");
          return getBuf("abcd1234abcd");
        }));
    write_.write(std::move(msg), Aead::AeadOptions());
  }
}

TEST_F(EncryptedRecordTest, TestWriteEmpty) {
  TLSMessage msg{ContentType::application_data, folly::IOBuf::create(0)};
  auto outBuf = write_.write(std::move(msg), Aead::AeadOptions());
  EXPECT_TRUE(outBuf.data->empty());
}

TEST_F(EncryptedRecordTest, TestWriteMaxSize) {
  write_.setMaxRecord(1900);

  TLSMessage msg{ContentType::application_data, IOBuf::create(2000)};
  msg.fragment->append(2000);
  memset(msg.fragment->writableData(), 0x1, msg.fragment->length());

  Sequence s;
  EXPECT_CALL(*writeAead_, _encrypt(_, _, _, _))
      .Times(2)
      .WillRepeatedly(Invoke([](std::unique_ptr<IOBuf>& /*buf*/,
                                const IOBuf*,
                                uint64_t,
                                Aead::AeadOptions) { return getBuf("aaaa"); }));
  write_.write(std::move(msg), Aead::AeadOptions());
}

TEST_F(EncryptedRecordTest, TestWriteMinSize) {
  TLSMessage msg{ContentType::application_data, IOBuf::create(1000)};
  msg.fragment->append(1000);
  memset(msg.fragment->writableData(), 0x1, msg.fragment->length());
  auto next = IOBuf::create(1000);
  next->append(1000);
  memset(next->writableData(), 0x2, next->length());
  msg.fragment->prependChain(std::move(next));

  Sequence s;
  EXPECT_CALL(*writeAead_, _encrypt(_, _, _, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        // one byte for footer
        EXPECT_EQ(buf->computeChainDataLength(), 1501);
        return getBuf("aaaa");
      }))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        // one byte for footer
        EXPECT_EQ(buf->computeChainDataLength(), 501);
        return getBuf("bbbb");
      }));
  write_.write(std::move(msg), Aead::AeadOptions());
}

TEST_F(EncryptedRecordTest, TestRecordState) {
  // Encrypted record layers keep track of sequence numbers
  auto testImpl = [](auto&& rlayer, auto&& aead) {
    auto state = rlayer.getRecordLayerState();
    EXPECT_FALSE(state.key.has_value());
    EXPECT_EQ(state.sequence.value(), 0);

    TrafficKey key;
    key.key = IOBuf::copyBuffer("key");
    key.iv = IOBuf::copyBuffer("iv");

    EXPECT_CALL(aead, getKey()).WillOnce(InvokeWithoutArgs([&] {
      return key.clone();
    }));
    rlayer.setSequenceNumber(10);

    state = rlayer.getRecordLayerState();
    EXPECT_TRUE(state.key.has_value());
    EXPECT_TRUE(
        folly::IOBufEqualTo{}(state.key->key, IOBuf::copyBuffer("key")));
    EXPECT_TRUE(folly::IOBufEqualTo{}(state.key->iv, IOBuf::copyBuffer("iv")));
    EXPECT_EQ(state.sequence.value(), 10);
  };

  testImpl(read_, *readAead_);
  testImpl(write_, *writeAead_);
}

TEST_F(EncryptedRecordTest, TestWritePaddingWithoutTailroom) {
  const uint16_t recordLength1 = 2000;
  const uint16_t recordLength2 = 2001;
  const uint16_t maxRecord = 0x4000;

  write_.setMaxRecord(maxRecord);
  write_.setBufAndPaddingPolicy(
      std::make_unique<BufAndModuloPaddingPolicy>(paddingModulo));

  const uint16_t expectedPadding1 = (maxRecord - recordLength1) % paddingModulo;
  const uint16_t expectedPadding2 = (maxRecord - recordLength2) % paddingModulo;

  TLSMessage msg1{ContentType::application_data, IOBuf::create(recordLength1)};
  msg1.fragment->append(recordLength1);
  memset(msg1.fragment->writableData(), 0x1, msg1.fragment->length());

  TLSMessage msg2{ContentType::application_data, IOBuf::create(recordLength2)};
  msg2.fragment->append(recordLength2);
  memset(msg2.fragment->writableData(), 0x1, msg2.fragment->length());

  EXPECT_CALL(*writeAead_, _encrypt(_, _, _, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf1,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        // Record size + 1 byte footer + padding
        EXPECT_EQ(
            buf1->computeChainDataLength(),
            recordLength1 + expectedPadding1 + 1);
        EXPECT_EQ(buf1->countChainElements(), 2);
        return getBuf("aaaa");
      }));
  write_.write(std::move(msg1), Aead::AeadOptions());

  EXPECT_CALL(*writeAead_, _encrypt(_, _, _, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf2,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        // Record size + 1 byte footer + padding
        EXPECT_EQ(
            buf2->computeChainDataLength(),
            recordLength2 + expectedPadding2 + 1);
        EXPECT_EQ(buf2->countChainElements(), 2);
        return getBuf("aaaa");
      }));
  write_.write(std::move(msg2), Aead::AeadOptions());

  // ensure the two message lengths are padded up the same value
  EXPECT_EQ(recordLength1 + expectedPadding1, recordLength2 + expectedPadding2);
}

TEST_F(EncryptedRecordTest, TestWritePaddingWithTailroom) {
  const uint16_t recordLength1 = 2000;
  const uint16_t recordLength2 = 2001;
  const uint16_t maxRecord = 0x4000;

  write_.setMaxRecord(maxRecord);
  write_.setBufAndPaddingPolicy(
      std::make_unique<BufAndModuloPaddingPolicy>(paddingModulo));

  const uint16_t expectedPadding1 = (maxRecord - recordLength1) % paddingModulo;
  const uint16_t expectedPadding2 = (maxRecord - recordLength2) % paddingModulo;

  TLSMessage msg1{
      ContentType::application_data, IOBuf::create(recordLength1 + 1000)};
  msg1.fragment->append(recordLength1);
  memset(msg1.fragment->writableData(), 0x1, msg1.fragment->length());

  TLSMessage msg2{
      ContentType::application_data, IOBuf::create(recordLength2 + 1000)};
  msg2.fragment->append(recordLength1);
  memset(msg2.fragment->writableData(), 0x1, msg2.fragment->length());

  EXPECT_CALL(*writeAead_, _encrypt(_, _, _, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf1,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        // Record size + 1 byte footer + padding
        EXPECT_EQ(
            buf1->computeChainDataLength(),
            recordLength1 + expectedPadding1 + 1);
        EXPECT_EQ(buf1->countChainElements(), 1);
        return getBuf("aaaa");
      }));
  write_.write(std::move(msg1), Aead::AeadOptions());

  EXPECT_CALL(*writeAead_, _encrypt(_, _, _, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf2,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        // Record size + 1 byte footer + padding
        EXPECT_EQ(
            buf2->computeChainDataLength(),
            recordLength2 + expectedPadding2 + 1);
        EXPECT_EQ(buf2->countChainElements(), 1);
        return getBuf("aaaa");
      }));
  write_.write(std::move(msg2), Aead::AeadOptions());

  // ensure the two message lengths are padded up the same value
  EXPECT_EQ(recordLength1 + expectedPadding1, recordLength2 + expectedPadding2);
}

TEST_F(EncryptedRecordTest, TestWritePaddingAtMaxRecord) {
  const uint16_t recordLength = 2000;

  write_.setMaxRecord(recordLength);
  write_.setBufAndPaddingPolicy(
      std::make_unique<BufAndModuloPaddingPolicy>(paddingModulo));

  TLSMessage msg{ContentType::application_data, IOBuf::create(recordLength)};
  msg.fragment->append(recordLength);
  memset(msg.fragment->writableData(), 0x1, msg.fragment->length());

  EXPECT_CALL(*writeAead_, _encrypt(_, _, _, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        // Record size + 1 byte footer + 0 padding (due to max record)
        EXPECT_EQ(buf->computeChainDataLength(), recordLength + 1);
        return getBuf("aaaa");
      }));
  write_.write(std::move(msg), Aead::AeadOptions());
}

TEST_F(EncryptedRecordTest, TestWritePaddingNearMaxRecord) {
  const uint16_t recordLength = 2000;
  const uint16_t truncatedPadding = 20;

  write_.setMaxRecord(recordLength + truncatedPadding);
  write_.setBufAndPaddingPolicy(
      std::make_unique<BufAndModuloPaddingPolicy>(paddingModulo));

  TLSMessage msg{ContentType::application_data, IOBuf::create(recordLength)};
  msg.fragment->append(recordLength);
  memset(msg.fragment->writableData(), 0x1, msg.fragment->length());

  EXPECT_CALL(*writeAead_, _encrypt(_, _, _, _))
      .WillOnce(Invoke([](std::unique_ptr<IOBuf>& buf,
                          const IOBuf*,
                          uint64_t,
                          Aead::AeadOptions) {
        // Record size + 1 byte footer + truncated padding (due to max record)
        EXPECT_EQ(
            buf->computeChainDataLength(), recordLength + truncatedPadding + 1);
        return getBuf("aaaa");
      }));
  write_.write(std::move(msg), Aead::AeadOptions());
}
} // namespace test
} // namespace fizz
