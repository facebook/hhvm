/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/fizz-config.h>

#include <fizz/crypto/aead/IOBufUtil.h>
#include <fizz/crypto/aead/test/Data.h>
#include <fizz/crypto/aead/test/TestUtil.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/record/Types.h>
#include <folly/ExceptionWrapper.h>
#include <folly/String.h>

#include <list>
#include <stdexcept>

using namespace folly;

namespace fizz {
using test::chunkIOBuf;
using test::createBufExact;
using test::toIOBuf;

namespace openssl {
namespace test {

constexpr size_t kHeadroom = 10;

class EVPCipherTest : public ::testing::TestWithParam<CipherParams> {};

std::unique_ptr<Aead> getTestCipher(const CipherParams& params) {
  std::unique_ptr<Aead> cipher = ::fizz::test::getCipher(params.cipher);

  TrafficKey trafficKey;
  trafficKey.key = toIOBuf(params.key);
  trafficKey.iv = toIOBuf(params.iv);
  cipher->setKey(std::move(trafficKey));
  return cipher;
}

std::list<Aead::AeadOptions> getOptionPairs() {
  std::list<Aead::AeadOptions> out;
  for (auto aOpt :
       {Aead::AllocationOption::Allow, Aead::AllocationOption::Deny}) {
    for (auto bOpt :
         {Aead::BufferOption::RespectSharedPolicy,
          Aead::BufferOption::AllowInPlace,
          Aead::BufferOption::AllowFullModification}) {
      out.push_back({bOpt, aOpt});
    }
  }
  return out;
}

std::unique_ptr<folly::IOBuf> callEncrypt(
    std::unique_ptr<Aead>& cipher,
    const CipherParams& params,
    std::unique_ptr<IOBuf> plaintext,
    Aead::BufferOption buffOption = Aead::BufferOption::RespectSharedPolicy,
    Aead::AllocationOption allocOption = Aead::AllocationOption::Allow,
    std::unique_ptr<IOBuf> aad = nullptr) {
  if (!aad && !params.aad.empty()) {
    aad = toIOBuf(params.aad);
  }

  auto origLength = plaintext->computeChainDataLength();

  auto out = cipher->encrypt(
      std::move(plaintext),
      aad.get(),
      params.seqNum,
      {buffOption, allocOption});
  bool valid = IOBufEqualTo()(toIOBuf(params.ciphertext), out);

  EXPECT_EQ(valid, params.valid);
  EXPECT_EQ(
      out->computeChainDataLength(), origLength + cipher->getCipherOverhead());
  return out;
}

std::unique_ptr<IOBuf> callDecrypt(
    std::unique_ptr<Aead>& cipher,
    const CipherParams& params,
    std::unique_ptr<IOBuf> ciphertext = nullptr,
    Aead::BufferOption buffOption = Aead::BufferOption::RespectSharedPolicy,
    Aead::AllocationOption allocOption = Aead::AllocationOption::Allow,
    bool throwExceptions = false,
    std::unique_ptr<IOBuf> aad = nullptr) {
  if (!ciphertext) {
    ciphertext = toIOBuf(params.ciphertext);
  }
  if (!aad && !params.aad.empty()) {
    aad = toIOBuf(params.aad);
  }
  try {
    auto origLength = ciphertext->computeChainDataLength();
    auto out = cipher->decrypt(
        std::move(ciphertext),
        aad.get(),
        params.seqNum,
        {buffOption, allocOption});
    EXPECT_TRUE(params.valid);
    EXPECT_TRUE(IOBufEqualTo()(toIOBuf(params.plaintext), out));
    EXPECT_EQ(
        out->computeChainDataLength(),
        origLength - cipher->getCipherOverhead());
    return out;
  } catch (const std::runtime_error&) {
    if (throwExceptions) {
      // Indicates test case wants to receive any exceptions thrown
      throw;
    } else {
      EXPECT_FALSE(params.valid);
    }
    return nullptr;
  }
}

TEST_P(EVPCipherTest, TestEncrypt) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto plaintext = toIOBuf(GetParam().plaintext);
    if (opts.allocOpt == Aead::AllocationOption::Deny) {
      EXPECT_THROW(
          callEncrypt(
              cipher,
              GetParam(),
              std::move(plaintext),
              opts.bufferOpt,
              opts.allocOpt),
          std::runtime_error);
    } else {
      // Buffer isn't shared, so all buffer option cases should be equal
      auto out = callEncrypt(
          cipher,
          GetParam(),
          std::move(plaintext),
          opts.bufferOpt,
          opts.allocOpt);
      EXPECT_EQ(out->headroom(), 0);
    }
  }
}

TEST_P(EVPCipherTest, TestEncryptWithTagRoom) {
  for (auto opts : getOptionPairs()) {
    // Behavior should be identical for all, as buffer is unshared with enough
    // room
    auto cipher = getTestCipher(GetParam());
    auto input = toIOBuf(GetParam().plaintext, 0, cipher->getCipherOverhead());
    auto out = callEncrypt(
        cipher, GetParam(), std::move(input), opts.bufferOpt, opts.allocOpt);
    EXPECT_FALSE(out->isChained());
  }
}

TEST_P(EVPCipherTest, TestEncryptReusedCipher) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto params = GetParam();
    if (opts.allocOpt == Aead::AllocationOption::Deny) {
      // Again, no tag room, so we expect these to throw
      EXPECT_THROW(
          callEncrypt(
              cipher,
              params,
              toIOBuf(params.plaintext),
              opts.bufferOpt,
              opts.allocOpt),
          std::runtime_error);
      EXPECT_THROW(
          callEncrypt(
              cipher,
              GetParam(),
              toIOBuf(params.plaintext),
              opts.bufferOpt,
              opts.allocOpt),
          std::runtime_error);
    } else {
      callEncrypt(
          cipher,
          params,
          toIOBuf(params.plaintext),
          opts.bufferOpt,
          opts.allocOpt);
      callEncrypt(
          cipher,
          GetParam(),
          toIOBuf(params.plaintext),
          opts.bufferOpt,
          opts.allocOpt);
    }
  }
}

TEST_P(EVPCipherTest, TestEncryptReusedCipherWithTagRoom) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto params = GetParam();
    callEncrypt(
        cipher,
        params,
        toIOBuf(params.plaintext, 0, cipher->getCipherOverhead()),
        opts.bufferOpt,
        opts.allocOpt);
    callEncrypt(
        cipher,
        GetParam(),
        toIOBuf(params.plaintext, 0, cipher->getCipherOverhead()),
        opts.bufferOpt,
        opts.allocOpt);
  }
}

TEST_P(EVPCipherTest, TestEncryptChunkedInput) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto inputLength = toIOBuf(GetParam().plaintext)->computeChainDataLength();
    for (size_t i = 2; i < inputLength; i++) {
      auto input = toIOBuf(GetParam().plaintext);
      auto chunkedInput = chunkIOBuf(std::move(input), i);
      if (opts.allocOpt == Aead::AllocationOption::Deny) {
        // No tag room
        EXPECT_THROW(
            callEncrypt(
                cipher,
                GetParam(),
                std::move(chunkedInput),
                opts.bufferOpt,
                opts.allocOpt),
            std::runtime_error);
      } else {
        callEncrypt(
            cipher,
            GetParam(),
            std::move(chunkedInput),
            opts.bufferOpt,
            opts.allocOpt);
      }
    }
  }
}

TEST_P(EVPCipherTest, TestEncryptChunkedInputWithEmpty) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto input = toIOBuf(GetParam().plaintext);
    auto chunkedInput = chunkIOBuf(std::move(input), 3);
    // add a zero length for the second node
    chunkedInput->appendChain(IOBuf::create(0));
    EXPECT_EQ(4, chunkedInput->countChainElements());
    if (opts.allocOpt == Aead::AllocationOption::Deny) {
      // no tag room
      EXPECT_THROW(
          callEncrypt(
              cipher,
              GetParam(),
              std::move(chunkedInput),
              opts.bufferOpt,
              opts.allocOpt),
          std::runtime_error);
    } else {
      callEncrypt(
          cipher,
          GetParam(),
          std::move(chunkedInput),
          opts.bufferOpt,
          opts.allocOpt);
    }
  }
}

TEST_P(EVPCipherTest, TestEncryptChunkedInputWithTagRoomHead) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto input = toIOBuf(GetParam().plaintext);
    auto overhead = cipher->getCipherOverhead();
    auto creator = [overhead](size_t len, size_t num) {
      if (num == 0) {
        // create a buffer w/ room for the tag
        auto result = createBufExact(len + overhead);
        result->reserve(0, overhead);
        return result;
      }
      return createBufExact(len);
    };
    auto chunkedInput = chunkIOBuf(std::move(input), 3, creator);
    // even though the head element has tailroom, we don't use it since it's
    // the last element that needs to have it for copying tag in directly
    if (opts.allocOpt == Aead::AllocationOption::Deny) {
      // Since we can't allocate room for the tag where we need it, expect to
      // throw
      EXPECT_THROW(
          callEncrypt(
              cipher,
              GetParam(),
              std::move(chunkedInput),
              opts.bufferOpt,
              opts.allocOpt),
          std::runtime_error);
    } else {
      auto out = callEncrypt(
          cipher,
          GetParam(),
          std::move(chunkedInput),
          opts.bufferOpt,
          opts.allocOpt);
      // We expect the first buffer to have the tailroom unused
      EXPECT_GE(out->tailroom(), overhead);
    }
  }
}

TEST_P(EVPCipherTest, TestEncryptChunkedInputWithTagRoomLast) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto input = toIOBuf(GetParam().plaintext);
    auto overhead = cipher->getCipherOverhead();
    size_t chunks = 3;
    auto creator = [=](size_t len, size_t num) {
      if (num == chunks - 1) {
        // create a buffer w/ room for the tag
        auto result = createBufExact(len + overhead);
        result->reserve(0, overhead);
        return result;
      }
      return createBufExact(len);
    };
    auto chunkedInput = chunkIOBuf(std::move(input), chunks, creator);
    auto lastTailRoom = chunkedInput->prev()->tailroom();
    auto numElements = chunkedInput->countChainElements();
    auto out = callEncrypt(
        cipher,
        GetParam(),
        std::move(chunkedInput),
        opts.bufferOpt,
        opts.allocOpt);
    // we expect the last element in the chain to have tailroom - overhead
    // left
    EXPECT_EQ(out->prev()->tailroom(), lastTailRoom - overhead);
    EXPECT_EQ(out->countChainElements(), numElements);
  }
}

TEST_P(EVPCipherTest, TestEncryptChunkedSharedInput) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto input = toIOBuf(GetParam().plaintext);
    auto chunkedInput = chunkIOBuf(std::move(input), 3);
    if (opts.allocOpt == Aead::AllocationOption::Deny) {
      // They'll all fail for lack of tag room
      EXPECT_THROW(
          callEncrypt(
              cipher,
              GetParam(),
              chunkedInput->clone(),
              opts.bufferOpt,
              opts.allocOpt),
          std::runtime_error);
    } else {
      auto originalHeadroom = chunkedInput->headroom();
      auto out = callEncrypt(
          cipher,
          GetParam(),
          chunkedInput->clone(),
          opts.bufferOpt,
          opts.allocOpt);
      if (opts.bufferOpt == Aead::BufferOption::RespectSharedPolicy) {
        // we expect headroom of record size and a single buffer in the
        // the chain. as it should be a new buffer, it should be unshared.
        EXPECT_EQ(out->headroom(), kHeadroom);
        EXPECT_FALSE(out->isChained());
        EXPECT_FALSE(out->isShared());
      } else {
        // We explicitly supported in-place edits. Since there isn't tailroom,
        // we expect identical behavior (new tag buffer)
        EXPECT_EQ(out->headroom(), originalHeadroom);
        EXPECT_TRUE(out->isChained());
        EXPECT_TRUE(out->isShared());
        EXPECT_EQ(
            out->countChainElements(), chunkedInput->countChainElements() + 1);
      }
    }
  }
}

TEST_P(EVPCipherTest, TestEncryptChunkedSharedInputWithTagRoom) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto input = toIOBuf(GetParam().plaintext);
    auto overhead = cipher->getCipherOverhead();
    size_t chunks = 3;
    auto creator = [=](size_t len, size_t num) {
      if (num == chunks - 1) {
        // create a buffer w/ room for the tag
        auto result = createBufExact(len + overhead);
        result->reserve(0, overhead);
        return result;
      }
      return createBufExact(len);
    };
    auto chunkedInput = chunkIOBuf(std::move(input), chunks, creator);
    if (opts.allocOpt == Aead::AllocationOption::Deny) {
      if (opts.bufferOpt != Aead::BufferOption::AllowFullModification) {
        // When denying allocation, edits must allow for growth
        EXPECT_THROW(
            callEncrypt(
                cipher,
                GetParam(),
                chunkedInput->clone(),
                opts.bufferOpt,
                opts.allocOpt),
            std::runtime_error);
      } else {
        auto originalHeadroom = chunkedInput->headroom();
        auto out = callEncrypt(
            cipher,
            GetParam(),
            chunkedInput->clone(),
            opts.bufferOpt,
            opts.allocOpt);
        // We expect that it edited in-place and grew the last buffer.
        EXPECT_EQ(out->headroom(), originalHeadroom);
        EXPECT_TRUE(out->isChained());
        EXPECT_TRUE(out->isShared());
        EXPECT_EQ(
            out->countChainElements(), chunkedInput->countChainElements());
        EXPECT_EQ(
            out->prev()->length(), chunkedInput->prev()->length() + overhead);
      }
    } else {
      auto originalHeadroom = chunkedInput->headroom();
      auto out = callEncrypt(
          cipher,
          GetParam(),
          chunkedInput->clone(),
          opts.bufferOpt,
          opts.allocOpt);
      if (opts.bufferOpt == Aead::BufferOption::RespectSharedPolicy) {
        // we expect headroom of record size and a single buffer in the
        // the chain. as it should be a new buffer, it should be unshared.
        EXPECT_EQ(out->headroom(), kHeadroom);
        EXPECT_FALSE(out->isChained());
        EXPECT_FALSE(out->isShared());
      } else if (opts.bufferOpt == Aead::BufferOption::AllowInPlace) {
        // We explicitly supported in-place edits, but not growth.
        // There should be a new buffer with the tag.
        EXPECT_EQ(out->headroom(), originalHeadroom);
        EXPECT_TRUE(out->isChained());
        EXPECT_TRUE(out->isShared());
        EXPECT_EQ(
            out->countChainElements(), chunkedInput->countChainElements() + 1);
        EXPECT_EQ(out->prev()->length(), overhead);
      } else {
        // Should be able to do it entirely in-place, growing the last buffer.
        EXPECT_EQ(out->headroom(), originalHeadroom);
        EXPECT_TRUE(out->isChained());
        EXPECT_TRUE(out->isShared());
        EXPECT_EQ(
            out->countChainElements(), chunkedInput->countChainElements());
        EXPECT_EQ(
            out->prev()->length(), chunkedInput->prev()->length() + overhead);
      }
    }
  }
}

TEST_P(EVPCipherTest, TestEncryptChunkedAad) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto aadLength = toIOBuf(GetParam().aad)->computeChainDataLength();
    for (size_t i = 2; i < aadLength; i++) {
      auto aad = toIOBuf(GetParam().aad);
      auto chunkedAad = chunkIOBuf(std::move(aad), i);
      if (opts.allocOpt == Aead::AllocationOption::Deny) {
        // No tag room
        EXPECT_THROW(
            callEncrypt(
                cipher,
                GetParam(),
                toIOBuf(GetParam().plaintext),
                opts.bufferOpt,
                opts.allocOpt,
                std::move(chunkedAad)),
            std::runtime_error);
      } else {
        callEncrypt(
            cipher,
            GetParam(),
            toIOBuf(GetParam().plaintext),
            opts.bufferOpt,
            opts.allocOpt,
            std::move(chunkedAad));
      }
    }
  }
}

TEST_P(EVPCipherTest, TestEncryptChunkedAadWithTagRoom) {
  for (auto opts : getOptionPairs()) {
    // Unique input with room for growth, should always succeed.
    auto cipher = getTestCipher(GetParam());
    auto aad = toIOBuf(GetParam().aad);
    auto chunkedAad = chunkIOBuf(std::move(aad), 3);
    callEncrypt(
        cipher,
        GetParam(),
        toIOBuf(GetParam().plaintext, 0, cipher->getCipherOverhead()),
        opts.bufferOpt,
        opts.allocOpt,
        std::move(chunkedAad));
  }
}

TEST_P(EVPCipherTest, TestDecrypt) {
  for (auto opts : getOptionPairs()) {
    // Should work for all modes (contiguous unshared buf)
    auto cipher = getTestCipher(GetParam());
    auto output = toIOBuf(GetParam().ciphertext);
    auto cipherLen = output->length();
    auto out =
        callDecrypt(cipher, GetParam(), nullptr, opts.bufferOpt, opts.allocOpt);
    if (out) {
      EXPECT_FALSE(out->isChained());
      EXPECT_FALSE(out->isShared());
      EXPECT_EQ(out->length(), cipherLen - cipher->getCipherOverhead());
    }
  }
}

TEST_P(EVPCipherTest, TestDecryptReusedCipher) {
  for (auto opts : getOptionPairs()) {
    // Same as before
    auto cipher = getTestCipher(GetParam());
    auto params = GetParam();
    callDecrypt(cipher, params, nullptr, opts.bufferOpt, opts.allocOpt);
    callDecrypt(cipher, GetParam(), nullptr, opts.bufferOpt, opts.allocOpt);
  }
}

TEST_P(EVPCipherTest, TestDecryptInputTooSmall) {
  for (auto opts : getOptionPairs()) {
    // This should behave identically (the input size check comes early)
    auto cipher = getTestCipher(GetParam());
    auto in = IOBuf::copyBuffer("in");
    auto paramsCopy = GetParam();
    paramsCopy.valid = false;
    callDecrypt(
        cipher, paramsCopy, std::move(in), opts.bufferOpt, opts.allocOpt);
  }
}

TEST_P(EVPCipherTest, TestDecryptWithChunkedInput) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto output = toIOBuf(GetParam().ciphertext);
    auto chunkedOutput = chunkIOBuf(std::move(output), 3);
    auto lastBufLength = chunkedOutput->prev()->length();
    if (opts.allocOpt == Aead::AllocationOption::Deny &&
        lastBufLength < cipher->getCipherOverhead()) {
      // If the last buf isn't big enough to hold the entire tag and allocation
      // isn't allowed, decrypt will fail
      EXPECT_THROW(
          callDecrypt(
              cipher,
              GetParam(),
              std::move(chunkedOutput),
              opts.bufferOpt,
              opts.allocOpt,
              true),
          std::runtime_error);
    } else {
      // In all other cases this should always succeed
      callDecrypt(
          cipher,
          GetParam(),
          std::move(chunkedOutput),
          opts.bufferOpt,
          opts.allocOpt);
    }
  }
}

TEST_P(EVPCipherTest, TestDecryptWithChunkedSharedInput) {
  for (auto opts : getOptionPairs()) {
    auto cipher = getTestCipher(GetParam());
    auto ciphertextLength =
        toIOBuf(GetParam().ciphertext)->computeChainDataLength();
    for (size_t i = 2; i < ciphertextLength; i++) {
      auto output = toIOBuf(GetParam().ciphertext);
      auto chunkedOutput = chunkIOBuf(std::move(output), i);
      auto lastBufLength = chunkedOutput->prev()->length();
      if (opts.allocOpt == Aead::AllocationOption::Deny &&
          (lastBufLength < cipher->getCipherOverhead() ||
           opts.bufferOpt == Aead::BufferOption::RespectSharedPolicy)) {
        EXPECT_THROW(
            callDecrypt(
                cipher,
                GetParam(),
                chunkedOutput->clone(),
                opts.bufferOpt,
                opts.allocOpt,
                true),
            std::runtime_error);
      } else {
        auto out = callDecrypt(
            cipher,
            GetParam(),
            chunkedOutput->clone(),
            opts.bufferOpt,
            opts.allocOpt);
        if (out) {
          // for valid cases:
          EXPECT_EQ(
              out->computeChainDataLength(),
              chunkedOutput->computeChainDataLength() -
                  cipher->getCipherOverhead());
          if (opts.bufferOpt != Aead::BufferOption::RespectSharedPolicy) {
            // In-place edit
            EXPECT_EQ(chunkedOutput->data(), out->data());
            EXPECT_TRUE(out->isChained());
            EXPECT_TRUE(out->isShared());
            EXPECT_EQ(
                out->countChainElements(), chunkedOutput->countChainElements());
          } else {
            // New buffer.
            EXPECT_NE(chunkedOutput->data(), out->data());
            EXPECT_FALSE(out->isChained());
            EXPECT_FALSE(out->isShared());
          }
        }
      }
    }
  }
}

TEST_P(EVPCipherTest, TestDecryptWithChunkedAad) {
  for (auto opts : getOptionPairs()) {
    // Behaviorally same as the regular decrypt test wrt variations
    auto cipher = getTestCipher(GetParam());
    auto aadLength = toIOBuf(GetParam().aad)->computeChainDataLength();
    for (size_t i = 2; i < aadLength; i++) {
      auto aad = toIOBuf(GetParam().aad);
      auto chunkedAad = chunkIOBuf(std::move(aad), i);
      callDecrypt(
          cipher,
          GetParam(),
          nullptr,
          opts.bufferOpt,
          opts.allocOpt,
          false,
          std::move(chunkedAad));
    }
  }
}

TEST_P(EVPCipherTest, TestTryDecrypt) {
  for (auto opts : getOptionPairs()) {
    // Should all behave identically, as ciphertext is unshared and contiguous
    auto cipher = getTestCipher(GetParam());
    auto out = cipher->tryDecrypt(
        toIOBuf(GetParam().ciphertext),
        toIOBuf(GetParam().aad).get(),
        GetParam().seqNum,
        std::move(opts));
    if (out) {
      EXPECT_TRUE(GetParam().valid);
      EXPECT_TRUE(IOBufEqualTo()(toIOBuf(GetParam().plaintext), *out));
    } else {
      EXPECT_FALSE(GetParam().valid);
    }
  }
}

TEST_P(EVPCipherTest, TestOutputBufferSizeOverflow) {
  for (auto opts : getOptionPairs()) {
    // new output buffer is only allocated when the plaintext input buffer is
    // shared and when we are allowed to allocate more memory
    if (opts.bufferOpt != Aead::BufferOption::RespectSharedPolicy ||
        opts.allocOpt != Aead::AllocationOption::Allow) {
      continue;
    }
    auto cipher = getTestCipher(GetParam());
    constexpr size_t kLargeHeadroom = 0xFFFFFFFFFFFFFFFF;
    cipher->setEncryptedBufferHeadroom(kLargeHeadroom);
    auto plaintext = toIOBuf(GetParam().plaintext);
    plaintext->markExternallyShared();

    EXPECT_TRUE(plaintext->isShared());
    EXPECT_THROW(
        callEncrypt(
            cipher,
            GetParam(),
            std::move(plaintext),
            opts.bufferOpt,
            opts.allocOpt),
        std::overflow_error);
  }
}

INSTANTIATE_TEST_SUITE_P(
    AESGCM128TestVectors,
    EVPCipherTest,
    ::testing::ValuesIn(kAes128GcmParams));

INSTANTIATE_TEST_SUITE_P(
    AESGCM256TestVectors,
    EVPCipherTest,
    ::testing::ValuesIn(kAes256GcmParams));

#if FOLLY_OPENSSL_HAS_CHACHA
INSTANTIATE_TEST_SUITE_P(
    ChaChaTestVectors,
    EVPCipherTest,
    ::testing::ValuesIn(kChaChaPoly1305Params));
#endif

#if !defined(OPENSSL_NO_OCB)
INSTANTIATE_TEST_SUITE_P(
    OCBTestVectors,
    EVPCipherTest,
    ::testing::ValuesIn(kAesOcbParams));
#endif

#if FIZZ_HAVE_LIBAEGIS
INSTANTIATE_TEST_SUITE_P(
    AEGIS128LTestVectors,
    EVPCipherTest,
    ::testing::ValuesIn(kAegis128LParams));
INSTANTIATE_TEST_SUITE_P(
    AEGIS256TestVectors,
    EVPCipherTest,
    ::testing::ValuesIn(kAegis256Params));
#endif
} // namespace test
} // namespace openssl
} // namespace fizz
