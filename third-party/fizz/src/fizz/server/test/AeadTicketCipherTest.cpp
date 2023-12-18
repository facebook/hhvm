/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/server/AeadTicketCipher.h>

#include <fizz/crypto/aead/test/Mocks.h>
#include <fizz/crypto/test/Mocks.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/clock/test/Mocks.h>
#include <fizz/protocol/test/Mocks.h>
#include <folly/String.h>

using namespace fizz::test;
using namespace testing;

static constexpr folly::StringPiece ticketSecret1{
    "90a791cf38c0b5c20447ef029ae1bc4bf3eecc2e85042174497671835ceaccd9"};
// secret: 13deec41c45b2f1c4f595ad5972d13047fba09031ba53140c751380e74114cc4
// salt: 4444444444444444444444444444444444444444444444444444444444444444
// hkdf output: c951156f3dcb1ab243a3f2c8e4346bec92cb25d241ae821484081388
static constexpr folly::StringPiece ticket1{
    "444444444444444444444444444444444444444444444444444444444444444400000000579bb5b10c83d7a581f6b8f7bd25acde3dabfe6f59e5147bde86681831"};
static constexpr folly::StringPiece ticket2{
    "444444444444444444444444444444444444444444444444444444444444444400000001f444b4f0a0d1dd8b26d3a0afa275b4f6956cfdce4857f9ec46177d0ff9"};

static constexpr folly::StringPiece ticketSecret2{
    "04de0343a34c12f17f8b9696443d55e533ca1eef92bdba6634a46b604e51436d"};
// secret: d2c07e1107d3024bd08ebf34d59b9726d05bd7082da80cbb1e90b879e0770b5f
// salt: 5cef31d266ca1fe1d634de9b95668d3d8895d4837d3ba81787185ff51c056e95
// hkdf output: f7d80b07236875b5a48bdc5bd4642a775c05c231b9507285675c1e0b
static constexpr folly::StringPiece ticket3{
    "5cef31d266ca1fe1d634de9b95668d3d8895d4837d3ba81787185ff51c056e95000000005d19a72a3becb5b063346fdf1ec6f9d9d4ddd82cb5f34a8ba0d19e4b69"};

// Uses context 'foobar'
static constexpr folly::StringPiece ticket4{
    "5cef31d266ca1fe1d634de9b95668d3d8895d4837d3ba81787185ff51c056e95000000005b2168cc0fda4f9987b5e9d045845ba4809ac5189158c578c0e5d11b00"};

static constexpr folly::StringPiece badTicket{
    "5d19a72a3becb5b061346fdf1ec6f9d9d4ddd82cb5f34a8ba0d19e4b69"};

namespace fizz {
namespace server {
namespace test {

class MockTicketCodecInstance {
 public:
  MOCK_METHOD(Buf, _encode, (ResumptionState & state), (const));
  MOCK_METHOD(
      ResumptionState,
      _decode,
      (Buf & encoded, const Factory& factory, const CertManager& certManager),
      (const));
};

class MockTicketCodec {
 public:
  static constexpr folly::StringPiece Label{"Mock Ticket Codec"};
  static Buf encode(ResumptionState state) {
    return instance->_encode(state);
  }
  static ResumptionState
  decode(Buf encoded, const Factory& factory, const CertManager& certManager) {
    return instance->_decode(encoded, factory, certManager);
  }
  static MockTicketCodecInstance* instance;
};
MockTicketCodecInstance* MockTicketCodec::instance;
constexpr folly::StringPiece MockTicketCodec::Label;

using TestAeadTicketCipher = Aead128GCMTicketCipher<MockTicketCodec>;

class AeadTicketCipherTest : public Test {
 public:
  AeadTicketCipherTest()
      : cipher_(
            std::make_shared<MockFactory>(),
            std::make_shared<CertManager>()) {}

  ~AeadTicketCipherTest() override = default;
  void SetUp() override {
    MockTicketCodec::instance = &codec_;
    clock_ = std::make_shared<MockClock>();
    policy_.setClock(clock_);
    cipher_.setPolicy(policy_);
  }

 protected:
  TicketPolicy policy_;
  TestAeadTicketCipher cipher_;
  MockTicketCodecInstance codec_;
  std::shared_ptr<MockClock> clock_;
  std::shared_ptr<Factory> factory_;
  std::shared_ptr<CertManager> certManager_;

  void rebuildCipher(std::string pskContext = "") {
    if (!pskContext.empty()) {
      cipher_ = TestAeadTicketCipher(
          std::make_shared<MockFactory>(),
          std::make_shared<CertManager>(),
          pskContext);
    } else {
      cipher_ = TestAeadTicketCipher(
          std::make_shared<MockFactory>(), std::make_shared<CertManager>());
    }
    cipher_.setPolicy(policy_);
    auto s1 = toIOBuf(ticketSecret1);
    auto s2 = toIOBuf(ticketSecret2);
    std::vector<folly::ByteRange> ticketSecrets{
        {s1->coalesce(), s2->coalesce()}};
    EXPECT_TRUE(cipher_.setTicketSecrets(std::move(ticketSecrets)));
  }

  void expectDecode() {
    EXPECT_CALL(codec_, _decode(_, _, _))
        .WillOnce(Invoke([](Buf& encoded,
                            const Factory& /* factory */,
                            const CertManager& /* certManager */) {
          EXPECT_TRUE(folly::IOBufEqualTo()(
              encoded, folly::IOBuf::copyBuffer("encodedticket")));
          return ResumptionState();
        }));
  }

  void checkUnsetEncrypt() {
    ResumptionState state;
    EXPECT_FALSE(cipher_.encrypt(std::move(state)).get().has_value());
  }

  void updatePolicy(
      std::chrono::seconds ticketValidity,
      folly::Optional<std::chrono::seconds> handshakeValidity = folly::none) {
    policy_.setTicketValidity(ticketValidity);
    if (handshakeValidity.has_value()) {
      policy_.setHandshakeValidity(*handshakeValidity);
    }
  }

  static ResumptionState makeState(
      std::chrono::system_clock::time_point handshakeTime) {
    ResumptionState state;
    state.handshakeTime = handshakeTime;
    return state;
  }
};

TEST_F(AeadTicketCipherTest, TestEncryptNoTicketSecrets) {
  checkUnsetEncrypt();
}

TEST_F(AeadTicketCipherTest, TestEncrypt) {
  useMockRandom();
  updatePolicy(std::chrono::seconds(5));
  rebuildCipher();
  EXPECT_CALL(codec_, _encode(_)).WillOnce(InvokeWithoutArgs([]() {
    return folly::IOBuf::copyBuffer("encodedticket");
  }));
  ResumptionState state;
  auto result = cipher_.encrypt(std::move(state)).get();
  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(folly::IOBufEqualTo()(result->first, toIOBuf(ticket1)));
  EXPECT_EQ(result->second, std::chrono::seconds(5));
}

TEST_F(AeadTicketCipherTest, TestHandshakeExpiration) {
  useMockRandom();
  updatePolicy(std::chrono::seconds(2), std::chrono::seconds(4));
  rebuildCipher();
  auto time = std::chrono::system_clock::now();
  EXPECT_CALL(*clock_, getCurrentTime()).WillOnce(Return(time));

  EXPECT_CALL(codec_, _encode(_)).WillOnce(InvokeWithoutArgs([]() {
    return folly::IOBuf::copyBuffer("encodedticket");
  }));
  EXPECT_CALL(codec_, _decode(_, _, _))
      .Times(2)
      .WillRepeatedly(InvokeWithoutArgs([time]() {
        ResumptionState res;
        res.handshakeTime = time;
        return res;
      }));
  auto result = cipher_.encrypt(makeState(time)).get();
  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(folly::IOBufEqualTo()(result->first, toIOBuf(ticket1)));
  EXPECT_EQ(result->second, std::chrono::seconds(2));
  EXPECT_CALL(*clock_, getCurrentTime())
      .WillOnce(Return(time + std::chrono::seconds(1)));
  auto decResult = cipher_.decrypt(result->first->clone()).get();
  EXPECT_EQ(decResult.first, PskType::Resumption);
  EXPECT_TRUE(decResult.second.has_value());
  EXPECT_CALL(*clock_, getCurrentTime())
      .WillOnce(Return(time + std::chrono::seconds(5)));
  auto badResult = cipher_.decrypt(result->first->clone()).get();
  EXPECT_EQ(badResult.first, PskType::Rejected);
  EXPECT_FALSE(badResult.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestTicketLifetime) {
  useMockRandom();
  updatePolicy(std::chrono::seconds(2), std::chrono::seconds(4));
  rebuildCipher();
  auto time = std::chrono::system_clock::now();

  EXPECT_CALL(codec_, _encode(_))
      .Times(2)
      .WillRepeatedly(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("encodedticket"); }));

  // At handshake time, expect ticket validity.
  EXPECT_CALL(*clock_, getCurrentTime()).WillOnce(Return(time));
  auto result = cipher_.encrypt(makeState(time)).get();
  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(folly::IOBufEqualTo()(result->first, toIOBuf(ticket1)));
  EXPECT_EQ(result->second, std::chrono::seconds(2));

  // At 3 seconds in, expect 1 second (remaining handshake validity)
  EXPECT_CALL(*clock_, getCurrentTime())
      .WillOnce(Return(time + std::chrono::seconds(3)));
  auto result2 = cipher_.encrypt(makeState(time)).get();
  EXPECT_TRUE(result2.has_value());
  EXPECT_TRUE(folly::IOBufEqualTo()(result2->first, toIOBuf(ticket1)));
  EXPECT_EQ(result2->second, std::chrono::seconds(1));

  // 5 seconds in, no longer valid. Expect none.
  EXPECT_CALL(*clock_, getCurrentTime())
      .WillOnce(Return(time + std::chrono::seconds(5)));
  auto result3 = cipher_.encrypt(makeState(time)).get();
  EXPECT_FALSE(result3.has_value());
}

TEST_F(AeadTicketCipherTest, TestEncryptExpiredHandshakeTicket) {
  useMockRandom();
  updatePolicy(std::chrono::hours(1), std::chrono::seconds(4));
  rebuildCipher();
  auto time = std::chrono::system_clock::now();
  EXPECT_CALL(*clock_, getCurrentTime()).WillOnce(Return(time));

  auto result =
      cipher_.encrypt(makeState(time - std::chrono::seconds(5))).get();
  EXPECT_FALSE(result.has_value());
}

TEST_F(AeadTicketCipherTest, TestEncryptTicketFromFuture) {
  useMockRandom();
  updatePolicy(std::chrono::seconds(2), std::chrono::seconds(4));
  rebuildCipher();
  auto time = std::chrono::system_clock::now();
  EXPECT_CALL(*clock_, getCurrentTime()).WillOnce(Return(time));

  EXPECT_CALL(codec_, _encode(_)).WillOnce(InvokeWithoutArgs([]() {
    return folly::IOBuf::copyBuffer("encodedticket");
  }));
  // Ticket was created in the future. Validity period should be equal
  // to maximum (as we can't be sure how old it really is)
  auto result =
      cipher_.encrypt(makeState(time + std::chrono::seconds(5))).get();
  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(folly::IOBufEqualTo()(result->first, toIOBuf(ticket1)));
  EXPECT_EQ(result->second, std::chrono::seconds(2));
}

TEST_F(AeadTicketCipherTest, TestDecryptNoTicketSecrets) {
  auto result = cipher_.decrypt(toIOBuf(ticket1)).get();
  EXPECT_EQ(result.first, PskType::Rejected);
  EXPECT_FALSE(result.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestDecryptFirst) {
  rebuildCipher();
  expectDecode();
  auto result = cipher_.decrypt(toIOBuf(ticket1)).get();
  EXPECT_EQ(result.first, PskType::Resumption);
  EXPECT_TRUE(result.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestDecryptSecond) {
  rebuildCipher();
  expectDecode();
  auto result = cipher_.decrypt(toIOBuf(ticket3)).get();
  EXPECT_EQ(result.first, PskType::Resumption);
  EXPECT_TRUE(result.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestDecryptWithContext) {
  rebuildCipher("foobar");
  expectDecode();
  auto result = cipher_.decrypt(toIOBuf(ticket4)).get();
  EXPECT_EQ(result.first, PskType::Resumption);
  EXPECT_TRUE(result.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestDecryptWithoutContext) {
  rebuildCipher();
  // Ticket 4 needs context 'foobar'
  auto result = cipher_.decrypt(toIOBuf(ticket4)).get();
  EXPECT_EQ(result.first, PskType::Rejected);
  EXPECT_FALSE(result.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestDecryptWithWrongContext) {
  rebuildCipher("barbaz");
  // barbaz =/= foobar
  auto result = cipher_.decrypt(toIOBuf(ticket4)).get();
  EXPECT_EQ(result.first, PskType::Rejected);
  EXPECT_FALSE(result.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestDecryptWithUnneededContext) {
  rebuildCipher("foobar");
  // Now test that ticket 3 with context 'foobar' doesn't work
  auto result = cipher_.decrypt(toIOBuf(ticket3)).get();
  EXPECT_EQ(result.first, PskType::Rejected);
  EXPECT_FALSE(result.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestDecryptSeqNum) {
  rebuildCipher();
  expectDecode();
  auto result = cipher_.decrypt(toIOBuf(ticket2)).get();
  EXPECT_EQ(result.first, PskType::Resumption);
  EXPECT_TRUE(result.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestDecryptFailed) {
  rebuildCipher();
  auto result = cipher_.decrypt(toIOBuf(badTicket)).get();
  EXPECT_EQ(result.first, PskType::Rejected);
  EXPECT_FALSE(result.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestDecryptTooShort) {
  rebuildCipher();
  auto result = cipher_.decrypt(folly::IOBuf::copyBuffer("short")).get();
  EXPECT_EQ(result.first, PskType::Rejected);
  EXPECT_FALSE(result.second.has_value());
}

TEST_F(AeadTicketCipherTest, TestUnsetTicketSecrets) {
  rebuildCipher();
  EXPECT_TRUE(cipher_.setTicketSecrets(std::vector<folly::ByteRange>()));
  checkUnsetEncrypt();
}

TEST_F(AeadTicketCipherTest, TestSetTicketSecretsTooShort) {
  folly::StringPiece tooShort{"short"};
  std::vector<folly::ByteRange> ticketSecrets{{tooShort}};
  EXPECT_FALSE(cipher_.setTicketSecrets(std::move(ticketSecrets)));
  checkUnsetEncrypt();
}
} // namespace test
} // namespace server
} // namespace fizz
