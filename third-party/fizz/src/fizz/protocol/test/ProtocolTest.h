/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/protocol/Actions.h>
#include <fizz/protocol/test/Matchers.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/record/test/Mocks.h>

namespace fizz {
namespace test {

constexpr ProtocolVersion TestProtocolVersion = ProtocolVersion::tls_1_3;

template <typename SM, typename Actions>
class ProtocolTest : public testing::Test {
 protected:
  using MutateState = folly::Function<void(typename SM::State&)>;

  template <typename... Args>
  void expectActions(const Actions& actions) {
    EXPECT_EQ(getNumActions<Args...>(actions, true), actions.size());
  }

  template <typename T>
  T expectAction(Actions& actions) {
    for (auto& action : actions) {
      auto val = action.template getType<T>();
      if (val) {
        return std::move(*val);
      }
    }
    throw std::runtime_error("did not find expected action");
  }

  template <typename T>
  T expectActionReversed(Actions& actions) {
    for (auto it = actions.rbegin(); it != actions.rend(); it++) {
      auto val = it->template getType<T>();
      if (val) {
        return std::move(*val);
      }
    }
    throw std::runtime_error("did not find expected action");
  }

  template <class T>
  void expectSecret(
      Actions& actions,
      T secretType,
      folly::ByteRange expectedSecret) {
    DerivedSecret expectedDerivedSecret(expectedSecret, SecretType(secretType));
    for (auto& action : actions) {
      auto trySecret = action.asSecretAvailable();
      if (trySecret) {
        bool match = trySecret->secret.secret == expectedDerivedSecret.secret &&
            trySecret->secret.type == expectedDerivedSecret.type;
        if (match) {
          return;
        }
      }
    }
    FAIL() << "Did not get expected secret";
  }

  template <typename T>
  T expectSingleAction(Actions actions) {
    EXPECT_EQ(actions.size(), 1);
    return expectAction<T>(actions);
  }

  void processStateMutations(Actions& actions) {
    for (auto& action : actions) {
      auto mutateState = action.asMutateState();
      if (mutateState) {
        (*mutateState)(state_);
      }
    }
  }

  template <typename T>
  uint32_t getNumActions(const Actions& actions, bool expectNonZero) {
    uint32_t count = 0;
    for (const auto& action : actions) {
      if (action.template getType<T>()) {
        count++;
      }
    }
    if (expectNonZero) {
      EXPECT_NE(count, 0);
    }
    return count;
  }

  template <typename T1, typename T2, typename... Args>
  uint32_t getNumActions(const Actions& actions, bool expectNonZero) {
    return getNumActions<T1>(actions, expectNonZero) +
        getNumActions<T2, Args...>(actions, expectNonZero);
  }

  void expectExceptionType(Actions& actions) {
    auto error = expectAction<ReportError>(actions);
  }

  template <typename ExceptionType>
  ExceptionType expectErrorAtEnd(
      Actions& actions,
      folly::Optional<AlertDescription> alert,
      std::string msg = "") {
    if (alert) {
      auto write = expectActionReversed<WriteToSocket>(actions);
      Alert a;
      a.description = *alert;
      auto buf = folly::IOBuf::copyBuffer("alert");
      buf->prependChain(encode(std::move(a)));
      EXPECT_TRUE(folly::IOBufEqualTo()(write.contents[0].data, buf));
    }
    auto error = expectActionReversed<ReportError>(actions);
    auto ex = error.error.template get_exception<ExceptionType>();
    EXPECT_NE(ex, nullptr);
    EXPECT_THAT(error.error.what().toStdString(), HasSubstr(msg));
    processStateMutations(actions);
    EXPECT_EQ(state_.state(), SM::StateEnum::Error);
    EXPECT_EQ(state_.readRecordLayer(), nullptr);
    EXPECT_EQ(state_.writeRecordLayer(), nullptr);
    return *ex;
  }

  template <typename ExceptionType>
  ExceptionType expectError(
      Actions& actions,
      folly::Optional<AlertDescription> alert,
      std::string msg = "") {
    if (alert) {
      expectActions<MutateState, ReportError, WriteToSocket>(actions);
      auto write = expectAction<WriteToSocket>(actions);
      Alert a;
      a.description = *alert;
      auto buf = folly::IOBuf::copyBuffer("alert");
      buf->prependChain(encode(std::move(a)));
      EXPECT_TRUE(folly::IOBufEqualTo()(write.contents[0].data, buf));
    } else {
      expectActions<MutateState, ReportError>(actions);
    }
    auto error = expectAction<ReportError>(actions);
    auto ex = error.error.template get_exception<ExceptionType>();
    EXPECT_NE(ex, nullptr);
    EXPECT_THAT(error.error.what().toStdString(), HasSubstr(msg));
    processStateMutations(actions);
    EXPECT_EQ(state_.state(), SM::StateEnum::Error);
    EXPECT_EQ(state_.readRecordLayer(), nullptr);
    EXPECT_EQ(state_.writeRecordLayer(), nullptr);
    return *ex;
  }

  void expectAeadCreation(std::map<std::string, MockAead**> keys) {
    EXPECT_CALL(*factory_, makeAead(_)).WillRepeatedly(InvokeWithoutArgs([=]() {
      auto ret = std::make_unique<MockAead>();
      EXPECT_CALL(*ret, _setKey(_))
          .WillOnce(Invoke([keys, ptr = ret.get()](TrafficKey& key) {
            *keys.at(key.key->clone()->moveToFbString().toStdString()) = ptr;
          }));
      return ret;
    }));
  }

  void expectAeadCreation(MockAead** clientAead, MockAead** serverAead) {
    return expectAeadCreation(
        {{"clientkey", clientAead}, {"serverkey", serverAead}});
  }

  void expectEncryptedReadRecordLayerCreation(
      MockEncryptedReadRecordLayer** recordLayer,
      MockAead** readAead,
      folly::ByteRange expectedBaseSecret,
      folly::Optional<bool> skipFailedDecryption = folly::none,
      Sequence* s = nullptr) {
    EXPECT_CALL(*factory_, makeEncryptedReadRecordLayer(_))
        .InSequence(s ? *s : Sequence())
        .WillOnce(Invoke([=](EncryptionLevel encryptionLevel) {
          auto ret =
              std::make_unique<MockEncryptedReadRecordLayer>(encryptionLevel);
          *recordLayer = ret.get();
          EXPECT_CALL(*ret, _setAead(_, _))
              .WillOnce(Invoke([=](folly::ByteRange baseSecret, Aead* aead) {
                EXPECT_TRUE(baseSecret == expectedBaseSecret);
                EXPECT_EQ(aead, *readAead);
              }));
          if (skipFailedDecryption.hasValue()) {
            EXPECT_CALL(
                *ret, setSkipFailedDecryption(skipFailedDecryption.value()));
          }
          return ret;
        }));
  }

  void expectEncryptedWriteRecordLayerCreation(
      MockEncryptedWriteRecordLayer** recordLayer,
      MockAead** writeAead,
      folly::ByteRange expectedBaseSecret,
      std::function<TLSContent(TLSMessage&, MockEncryptedWriteRecordLayer*)>
          expectedWrite = nullptr,
      Sequence* s = nullptr) {
    EXPECT_CALL(*factory_, makeEncryptedWriteRecordLayer(_))
        .InSequence(s ? *s : Sequence())
        .WillOnce(Invoke([=](EncryptionLevel encryptionLevel) {
          auto ret =
              std::make_unique<MockEncryptedWriteRecordLayer>(encryptionLevel);
          ret->setDefaults();
          *recordLayer = ret.get();
          EXPECT_CALL(*ret, _setAead(_, _))
              .WillOnce(Invoke([=](folly::ByteRange baseSecret, Aead* aead) {
                EXPECT_TRUE(baseSecret == expectedBaseSecret);
                EXPECT_EQ(aead, *writeAead);
              }));
          if (expectedWrite) {
            EXPECT_CALL(*ret, _write(_, _))
                .WillOnce(
                    Invoke([writeFunc = std::move(expectedWrite),
                            writeRecord = *recordLayer](
                               TLSMessage& msg, Aead::AeadOptions) mutable {
                      return writeFunc(msg, writeRecord);
                    }));
          }
          return ret;
        }));
  }

  typename SM::State state_;
  MockFactory* factory_;
};
} // namespace test
} // namespace fizz
