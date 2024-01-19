/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/fizz-config.h>

#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/AESGCM256.h>
#include <fizz/crypto/aead/AESOCB128.h>
#include <fizz/crypto/aead/ChaCha20Poly1305.h>
#include <fizz/crypto/aead/IOBufUtil.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/crypto/aead/test/TestUtil.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/record/Types.h>
#include <folly/ExceptionWrapper.h>
#include <folly/String.h>

#include <list>
#include <stdexcept>

using namespace folly;

namespace fizz {
namespace test {

struct CipherParams {
  std::string key;
  std::string iv;
  uint64_t seqNum;
  std::string aad;
  std::string plaintext;
  std::string ciphertext;
  bool valid;
  CipherSuite cipher;
};

constexpr size_t kHeadroom = 10;

class EVPCipherTest : public ::testing::TestWithParam<CipherParams> {};

std::unique_ptr<Aead> getTestCipher(const CipherParams& params) {
  std::unique_ptr<Aead> cipher = getCipher(params.cipher);

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

// Adapted from draft-thomson-tls-tls13-vectors
INSTANTIATE_TEST_SUITE_P(
    AESGCM128TestVectors,
    EVPCipherTest,
    ::testing::Values(
        CipherParams{
            "87f6c12b1ae8a9b7efafc65af0f5c994",
            "479e25839c19e0476f95a6f5",
            1,
            "",
            "010015",
            "9d4db5ecd768198892531eebac72cf1d477dd0",
            true,
            CipherSuite::TLS_AES_128_GCM_SHA256},
        CipherParams{
            "911dc107aa6eccb6706bdcc37e76a07a",
            "11c7fa13e9499ed042b09e57",
            0,
            "",
            "14000020de15cbc8c62d0e6fef73a6d4e70e5c372c2b94fe08ea40d11166a7e6c967ba9c16",
            "56a21739148c898fe807026a179d59202647a3b1e01267a3883cf5f69fd233f63ff12c1c71b4c8f3d6086affb49621f96b842e1d35",
            true,
            CipherSuite::TLS_AES_128_GCM_SHA256},
        CipherParams{
            "a0f49e7076cae6eb25ca23a2da0eaf12",
            "3485d33f22128dff91e47062",
            0,
            "",
            "41424344454617",
            "92fdec5c241e994fb7d889e1b61d1db2b9be6777f5a393",
            true,
            CipherSuite::TLS_AES_128_GCM_SHA256},
        CipherParams{"fda2a4404670808f4937478b8b6e3fe1", "b5f3a3fae1cb25c9dcd73993", 0, "", "0800001e001c000a00140012001d00170018001901000101010201030104000000000b0001b9000001b50001b0308201ac30820115a003020102020102300d06092a864886f70d01010b0500300e310c300a06035504031303727361301e170d3136303733303031323335395a170d3236303733303031323335395a300e310c300a0603550403130372736130819f300d06092a864886f70d010101050003818d0030818902818100b4bb498f8279303d980836399b36c6988c0c68de55e1bdb826d3901a2461eafd2de49a91d015abbc9a95137ace6c1af19eaa6af98c7ced43120998e187a80ee0ccb0524b1b018c3e0b63264d449a6d38e22a5fda430846748030530ef0461c8ca9d9efbfae8ea6d1d03e2bd193eff0ab9a8002c47428a6d35a8d88d79f7f1e3f0203010001a31a301830090603551d1304023000300b0603551d0f0404030205a0300d06092a864886f70d01010b05000381810085aad2a0e5b9276b908c65f73a7267170618a54c5f8a7b337d2df7a594365417f2eae8f8a58c8f8172f9319cf36b7fd6c55b80f21a03015156726096fd335e5e67f2dbf102702e608ccae6bec1fc63a42a99be5c3eb7107c3c54e9b9eb2bd5203b1c3b84e0a8b2f759409ba3eac9d91d402dcc0cc8f8961229ac9187b42b4de100000f000084080400804547d6168f2510c550bd949cd2bc631ff134fa10a827ff69b166a6bd95e249ed0daf571592ebbe9ff13de6b03acc218146781f693b5a692b7319d74fd2e53b6a2df0f6785d624f024a44030ca00b869ae81a532b19e47e525ff4a62c51a5889eb565fee268590d8a3ca3c1bc3bd5404e39720ca2eaee308f4e0700761e986389140000209efee03ebffbc0dc23d26d958744c09e3000477eff7ae3148a50e5670013aaaa16", "c1e631f81d2af221ebb6a957f58f3ee266272635e67f99a752f0df08adeb33bab8611e55f33d72cf84382461a8bfe0a659ba2dd1873f6fcc707a9841cefc1fb03526b9ca4fe343e5805e95a5c01e56570638a76a4bc8feb07be879f90568617d905fecd5b1619fb8ec4a6628d1bb2bb224c490ff97a6c0e9acd03604bc3a59d86bdab4e084c1c1450f9c9d2afeb172c07234d739868ebd62de2060a8de989414a82920dacd1cac0c6e72ecd7f4018574ceaca6d29f361bc37ee2888b8e302ca9561a9de9163edfa66badd4894884c7b359bcacae5908051b37952e10a45fe73fda126ebd67575f1bed8a992a89474d7dec1eed327824123a414adb66d5ef7d0836ff98c2cdd7fb0781e192bf0c7568bf7d890a51c332879b5037b212d622412ca48e8323817bd6d746eef683845cec4e3ef64b3a18fcce513ea951f3366693a7ff490d09d08ab1f63e13625a545961599c0d9c7a099d1163cad1b9bcf8e917d766b98853ef6877834f891df16be1fcc9c18ea1882ea3f1f4b64358e1b146cebfb3e02e153fdb73af2693f22c6f593fa475380ba6611740ad20e319a654ac5684775236162e8447ed808861bfbda6e18ec97ae090bf703475cfb90fe20a3c55bef6f5eba6e6a1da6a1996b8bde42180608ca2279def8e8153895cc850db6420561c04b5729cc6883436ea02ee07eb9baee2fb3a9e1bbda8730d6b220576e24df70af6928eb865fee8a1d1c0f1818aca68d5002ae4c65b2f49c9e6e21dcf76784adbd0e887a36832ef85beb10587f16c6ffe60d7451059ec7f1014c3efe19e56aedb5ad31a9f29dc4458cfbf0c7070c175dcad46e1675226b47c071aad3172ebd33e45d741cb91253a01a69ae3cc292bce9c03246ac951e45e97ebf04a9d51fab5cf06d9485cce746b1c077be69ad153f1656ef89fc7d1ed8c3e2da7a2", true, CipherSuite::TLS_AES_128_GCM_SHA256},
        CipherParams{
            "a0f49e7076cbe6eb25ca23a2da0eaf12",
            "3485d33f22128dff91e47062",
            0,
            "",
            "41424344454617",
            "92fdec5c241e994fb7d889e1b61d1db2b9be6777f5a393",
            false,
            CipherSuite::TLS_AES_128_GCM_SHA256},
        CipherParams{
            "a0f49e7076cae6eb25ca23a2da0eaf12",
            "3485d33f22128dff91e47062",
            0,
            "",
            "41424344454617",
            "92fdec",
            false,
            CipherSuite::TLS_AES_128_GCM_SHA256},
        CipherParams{
            "AD7A2BD03EAC835A6F620FDCB506B345",
            "12153524C0895E81B2C28465",
            0,
            "D609B1F056637A0D46DF998D88E52E00B2C2846512153524C0895E81",
            "08000F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F303132333435363738393A0002",
            "701AFA1CC039C0D765128A665DAB69243899BF7318CCDC81C9931DA17FBE8EDD7D17CB8B4C26FC81E3284F2B7FBA713D4F8D55E7D3F06FD5A13C0C29B9D5B880",
            true,
            CipherSuite::TLS_AES_128_GCM_SHA256},
        CipherParams{
            "AD7A2BD03EAC835A6F620FDCB506B345",
            "12153524C0895E81B2C28465",
            0,
            "D609B1F056637A1D46DF998D88E52E00B2C2846512153524C0895E81",
            "08000F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F303132333435363738393A0002",
            "701AFA1CC039C0D765128A665DAB69243899BF7318CCDC81C9931DA17FBE8EDD7D17CB8B4C26FC81E3284F2B7FBA713D4F8D55E7D3F06FD5A13C0C29B9D5B880",
            false,
            CipherSuite::TLS_AES_128_GCM_SHA256}));

INSTANTIATE_TEST_SUITE_P(
    AESGCM256TestVectors,
    EVPCipherTest,
    ::testing::Values(
        CipherParams{
            "E3C08A8F06C6E3AD95A70557B23F75483CE33021A9C72B7025666204C69C0B72",
            "12153524C0895E81B2C28465",
            0,
            "D609B1F056637A0D46DF998D88E52E00B2C2846512153524C0895E81",
            "08000F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F303132333435363738393A0002",
            "E2006EB42F5277022D9B19925BC419D7A592666C925FE2EF718EB4E308EFEAA7C5273B394118860A5BE2A97F56AB78365CA597CDBB3EDB8D1A1151EA0AF7B436",
            true,
            CipherSuite::TLS_AES_256_GCM_SHA384},
        CipherParams{
            "E3C08A8F06C6E3AD95A70557B23F75483CE33021A9C72B7025666204C69C0B72",
            "12153524C0895E81B2C28465",
            0,
            "D609B1F056637A0D46DF998D88E52E00B2C2846512153524C0895E81",
            "08000F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F303132333435363738393A0002",
            "E2006EB42F5277022D9B19925BC419D7A592666C925FE2EF718EB4E308EFEAA7C5273B394118860A5BE2A97F56AB78365CA597CDBB3EDB8D1A1151EA1AF7B436",
            false,
            CipherSuite::TLS_AES_256_GCM_SHA384}));

#if FOLLY_OPENSSL_HAS_CHACHA
// Adapted from libressl's chacha20-poly1305 aead tests
INSTANTIATE_TEST_SUITE_P(
    ChaChaTestVectors,
    EVPCipherTest,
    ::testing::
        Values(
            CipherParams{
                "9a97f65b9b4c721b960a672145fca8d4e32e67f9111ea979ce9c4826806aeee6",
                "000000003de9c0da2bd7f91e",
                0,
                "",
                "",
                "5a6e21f4ba6dbee57380e79e79c30def",
                true,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
            CipherParams{
                "4290bcb154173531f314af57f3be3b5006da371ece272afa1b5dbdd1100a1007",
                "00000000cd7cf67be39c794a",
                0,
                "",
                "86d09974840bded2a5ca",
                "e3e446f7ede9a19b62a4dc8dae9a28bb548811461f49f8cec5ae",
                true,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
            CipherParams{
                "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f",
                "070000004041424344454647",
                0,
                "",
                "4c616469657320616e642047656e746c656d656e206f662074686520636c617373206f66202739393a204966204920636f756c64206f6666657220796f75206f6e6c79206f6e652074697020666f7220746865206675747572652c2073756e73637265656e20776f756c642062652069742e",
                "d31a8d34648e60db7b86afbc53ef7ec2a4aded51296e08fea9e2b5a736ee62d63dbea45e8ca9671282fafb69da92728b1a71de0a9e060b2905d6a5b67ecd3b3692ddbd7f2d778b8c9803aee328091b58fab324e4fad675945585808b4831d7bc3ff4def08e4b7a9de576d26586cec64b61166a23a4681fd59456aea1d29f82477216",
                true,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
            CipherParams{
                "1c9240a5eb55d38af333888604f6b5f0473917c1402b80099dca5cbc207075c0",
                "000000000102030405060708",
                0,
                "",
                "496e7465726e65742d4472616674732061726520647261667420646f63756d656e74732076616c696420666f722061206d6178696d756d206f6620736978206d6f6e74687320616e64206d617920626520757064617465642c207265706c616365642c206f72206f62736f6c65746564206279206f7468657220646f63756d656e747320617420616e792074696d652e20497420697320696e617070726f70726961746520746f2075736520496e7465726e65742d447261667473206173207265666572656e6365206d6174657269616c206f7220746f2063697465207468656d206f74686572207468616e206173202fe2809c776f726b20696e2070726f67726573732e2fe2809d",
                "64a0861575861af460f062c79be643bd5e805cfd345cf389f108670ac76c8cb24c6cfc18755d43eea09ee94e382d26b0bdb7b73c321b0100d4f03b7f355894cf332f830e710b97ce98c8a84abd0b948114ad176e008d33bd60f982b1ff37c8559797a06ef4f0ef61c186324e2b3506383606907b6a7c02b0f9f6157b53c867e4b9166c767b804d46a59b5216cde7a4e99040c5a40433225ee282a1b0a06c523eaf4534d7f83fa1155b0047718cbc546a0d072b04b3564eea1b422273f548271a0bb2316053fa76991955ebd63159434ecebb4e466dae5a1073a6727627097a1049e617d91d361094fa68f0ff77987130305beaba2eda04df997b714d6c6f2c29a6ad5cb4022b02709b6e3570b1acaaf1f24f2a644f01acd12b",
                true,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
            CipherParams{
                "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f",
                "a0a1a2a31011121314151617",
                0,
                "",
                "45000054a6f200004001e778c6336405c000020508005b7a3a080000553bec100007362708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363701020204",
                "24039428b97f417e3c13753a4f05087b67c352e6a7fab1b982d466ef407ae5c614ee8099d52844eb61aa95dfab4c02f72aa71e7c4c4f64c9befe2facc638e8f3cbec163fac469b502773f6fb94e664da9165b82829f641e07e236714fca1ccb75ab26d5f253185e6",
                true,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
            CipherParams{
                "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f",
                "a0a1a2a31011121314151617",
                0,
                "",
                "0000000c000040010000000a00",
                "610394701f8d017f7c129248895c5d2b5fa5a4723e5c38e903e5178a10",
                true,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
            CipherParams{
                "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f",
                "a0a1a2a31011121314151617",
                0,
                "",
                "0000000c000040010000000a00",
                "610394701f8d017f7c129248890c5d2b5fa5a4723e5c38e903e5178a10",
                false,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
            CipherParams{
                "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f",
                "a0a1a2a31011121314151617",
                0,
                "",
                "0000000c000040010000000a00",
                "610394701f8d017f7c129248",
                false,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
            CipherParams{
                "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f",
                "070000004041424344454647",
                0,
                "50515253c0c1c2c3c4c5c6c7",
                "4c616469657320616e642047656e746c656d656e206f662074686520636c617373206f66202739393a204966204920636f756c64206f6666657220796f75206f6e6c79206f6e652074697020666f7220746865206675747572652c2073756e73637265656e20776f756c642062652069742e",
                "d31a8d34648e60db7b86afbc53ef7ec2a4aded51296e08fea9e2b5a736ee62d63dbea45e8ca9671282fafb69da92728b1a71de0a9e060b2905d6a5b67ecd3b3692ddbd7f2d778b8c9803aee328091b58fab324e4fad675945585808b4831d7bc3ff4def08e4b7a9de576d26586cec64b61161ae10b594f09e26a7e902ecbd0600691",
                true,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
            CipherParams{
                "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f",
                "070000004041424344454647",
                0,
                "51515253c0c1c2c3c4c5c6c7",
                "4c616469657320616e642047656e746c656d656e206f662074686520636c617373206f66202739393a204966204920636f756c64206f6666657220796f75206f6e6c79206f6e652074697020666f7220746865206675747572652c2073756e73637265656e20776f756c642062652069742e",
                "d31a8d34648e60db7b86afbc53ef7ec2a4aded51296e08fea9e2b5a736ee62d63dbea45e8ca9671282fafb69da92728b1a71de0a9e060b2905d6a5b67ecd3b3692ddbd7f2d778b8c9803aee328091b58fab324e4fad675945585808b4831d7bc3ff4def08e4b7a9de576d26586cec64b61161ae10b594f09e26a7e902ecbd0600691",
                false,
                CipherSuite::TLS_CHACHA20_POLY1305_SHA256}));
#endif
#if FOLLY_OPENSSL_IS_110 && !defined(OPENSSL_NO_OCB)
// Adapted from openssl's evptests.txt AES OCB Test vectors
INSTANTIATE_TEST_SUITE_P(
    OCBTestVectors,
    EVPCipherTest,
    ::testing::Values(
        CipherParams{
            "000102030405060708090A0B0C0D0E0F",
            "000102030405060708090A0B",
            0,
            "0001020304050607",
            "0001020304050607",
            "92B657130A74B85A16DC76A46D47E1EAD537209E8A96D14E",
            true,
            CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL},
        CipherParams{
            "000102030405060708090A0B0C0D0E0F",
            "000102030405060708090A0B",
            0,
            "0001020304050607",
            "0001020304050607",
            "82B657130A74B85A16DC76A46D47E1EAD537209E8A96D14E",
            false,
            CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL},
        CipherParams{
            "000102030405060708090A0B0C0D0E0F",
            "000102030405060708090A0B",
            0,
            "000102030405060708090A0B0C0D0E0F",
            "000102030405060708090A0B0C0D0E0F",
            "BEA5E8798DBE7110031C144DA0B26122776C9924D6723A1FC4524532AC3E5BEB",
            true,
            CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL},
        CipherParams{
            "000102030405060708090A0B0C0D0E0F",
            "000102030405060708090A0B",
            0,
            "000102030405060708090A0B0C0D0E0F",
            "000102030405060708090A0B0C0D0E0F",
            "CEA5E8798DBE7110031C144DA0B26122776C9924D6723A1FC4524532AC3E5BEB",
            false,
            CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL},
        CipherParams{
            "000102030405060708090A0B0C0D0E0F",
            "000102030405060708090A0B",
            0,
            "000102030405060708090A0B0C0D0E0F1011121314151617",
            "000102030405060708090A0B0C0D0E0F1011121314151617",
            "BEA5E8798DBE7110031C144DA0B26122FCFCEE7A2A8D4D485FA94FC3F38820F1DC3F3D1FD4E55E1C",
            true,
            CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL},
        CipherParams{
            "000102030405060708090A0B0C0D0E0F",
            "000102030405060708090A0B",
            0,
            "000102030405060708090A0B0C0D0E0F1011121314151617",
            "000102030405060708090A0B0C0D0E0F1011121314151617",
            "BFA5E8798DBE7110031C144DA0B26122FCFCEE7A2A8D4D485FA94FC3F38820F1DC3F3D1FD4E55E1C",
            false,
            CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL},
        CipherParams{
            "000102030405060708090A0B0C0D0E0F",
            "000102030405060708090A0B",
            0,
            "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F2021222324252627",
            "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F2021222324252627",
            "BEA5E8798DBE7110031C144DA0B26122CEAAB9B05DF771A657149D53773463CB68C65778B058A635659C623211DEEA0DE30D2C381879F4C8",
            true,
            CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL}));
#endif

#if FIZZ_BUILD_AEGIS
// Adapted from libsodium's aegis testing values
INSTANTIATE_TEST_SUITE_P(
    AEGIS128LTestVectors,
    EVPCipherTest,
    ::testing::Values(
        CipherParams{
            "54662e55bb4771f9711fe5301d7412fe",
            "e51d417ab10a2931d8d22a9fffb98e3a",
            0,
            "3b762e3ab5d06cb2896b852ea70303f289f2775401b7808e30272f",
            "04f672f8cdb3e71d032d52c064bc33ecf8aad3d40c41d5806cc306766c057c50b500af5c550d076d34cc3a74a2b4bed195ffa3e8eddf953aefe9aed2bc14349c700ab7e4cb974fb31615a9ff70fb44307055523ab378b133fefc883013ce23bb01b23aeda15f85e65cdf02a291a0454900cb261872d5205737fd7410",
            "d6736371f35eb067244dd7963ad2e0cd3949452cbd4c220be55082498ed3b230f579d78844311652a9958e82f172bb8072c4b1114ec531a6ccb340ddd86caf32a0d4c9c45738e9ec9c0d9154612f7d90465f3a277bebd667c0af0edb6935d8dffbdee96c1a96e4c4318f5d3bc90c1c8d5729e1a402f765bdc9b26b0853c2fd22b035bf3f3658ede47ef11b9d",
            true,
            CipherSuite::TLS_AEGIS_128L_SHA256},
        CipherParams{
            "46a5c72e03d900b48f829df00ecb88b9",
            "b25187e4b77b6770c35c7a962584597d",
            0,
            "b73c81239e01cd81b0de13247ca4e3528b87f3078e2b674a667430b1dbdc3e93657131e654a4182b4c4ab01a33b36e946f1fcc55aab06fc6f56d",
            "fc8083311b38a80c04e57d069661b273264310906781eb7e4e44c6416f7336267674a44a7c54ed6361b43ef9500514e5d9e71f8b5c33aece756b64f3ed011922facbec7c3ffd27d01a853435bde551372806bd0c",
            "51189448af53ae3630c06a167ceefe6b9b5eba746fb9b53f4b3104d2b15b6020fa8998e182eb9c9d6b6463939e50723780f983733206ae6f11b986d95abe83555e64f8d3242d7e8055fcb8e2df8e41d318f06728f9b7e561ac0316d2ed7debc2484cc3e1",
            true,
            CipherSuite::TLS_AEGIS_128L_SHA256},
        CipherParams{
            "e343d75de99e6d73543968437d3dcf6a",
            "317a5808ed5debf6f527a780e0896b2d",
            0,
            "323094c01e",
            "247045cb40dea9c514a885444c526ac867b1b80e4728a23b63f596",
            "18cb5d2fc5e27bdda5ba16f1320da42049759368548e5bd96f2dbc1659e7d7193b12b4c90ba1e4314ef055",
            true,
            CipherSuite::TLS_AEGIS_128L_SHA256},
        CipherParams{
            "7db9c2721a03931c880f9e714bbf2211",
            "27f642398299ada7fdda1895ee4589f0",
            0,
            "6dd5e43033fa6f021059a353edaf1f870387693054d0a2360fd1f6941a68f48ba972a1bc0816a446a6186e4a9a2f9df556bf709470137b8e60d9daa2",
            "dc5180954df0c3391a60b44cbf70aee72b7dbb2addc90a0bf2ceac6113287eb501fe1ea9f4c51822664b82fe0279b039f4",
            "c8a7d9131cebfa5388003cc30deac523aa9b09d148affff06ba40400e09ca900db770e07cedf5cd0647f6723c810ffcb596cac51edd6f49cd7be0010a3ac29e704",
            true,
            CipherSuite::TLS_AEGIS_128L_SHA256},
        CipherParams{
            "00000000000000000000000000000000",
            "00000000000000000000000000000000",
            0,
            "6dd5e43033fa6f021059a353edaf1f870387693054d0a2360fd1f6941a68f48ba972a1bc0816a446a6186e4a9a2f9df556bf709470137b8e60d9daa2",
            "dc5180954df0c3391a60b44cbf70aee72b7dbb2addc90a0bf2ceac6113287eb501fe1ea9f4c51822664b82fe0279b039f4",
            "c8a7d9131cebfa5388003cc30deac523aa9b09d148affff06ba40400e09ca900db770e07cedf5cd0647f6723c810ffcb596cac51edd6f49cd7be0010a3ac29e704",
            false,
            CipherSuite::TLS_AEGIS_128L_SHA256}));
INSTANTIATE_TEST_SUITE_P(
    AEGIS256TestVectors,
    EVPCipherTest,
    ::testing::Values(
        CipherParams{
            "7083505997f52fdf86548d86ee87c1429ed91f108cd56384dc840269ef7fdd73",
            "18cd778e6f5b1d35d4ca975fd719a17aaf22c3eba01928b6a78bac5810c92c75",
            0,
            "af5b16a480e6a1400be15c8e6b194c2aca175e3b5c3f3fbbeca865f9390a",
            "5d6691271eb1b2261d1b34fa7560e274b83373343c2e49b2b6a82bc0f20cee85cd608d195c1a16679d720441c95fae86631f3f2cd27f38f71cedc79aaca7fdddbd4da4eeb97632366db65ca21acd85b41fd1a9de688bddff433a4757eb084e6816dbc8ff93f5995804",
            "0943a3e659b86e267ffea969ddd6d6d63aa35d1a1f31fb6f47205104b132da65799cc64cc9f66ffa5ec479550c2c5dfa006f827ef02e3ab4dae3446bf93ccb5c17e1ec0393f161fca94f2944d041f162e9c964558b6b57d3bb393b9743b1f8338ff878a154800fd16c6eacac942353072bdeb9fcf85e5b6c04",
            true,
            CipherSuite::TLS_AEGIS_256_SHA512},
        CipherParams{
            "c88bb05b2aec1218e1a5026511e6d44de7bd502588e9e2a01591b39c5ead76ff",
            "4a485f226a73f0c4e16242e8234841cdf6af1771eb278e7f35428d03eb5b4cf0",
            0,
            "38a9809dbdd2579010d38bf5314f255b",
            "2a4c06941ec356390542d7d7833fd68fc85a00c0452281f87dee6f10180d02182791232c7007fde35dfd5a901afa896296f9f344db717994d078fbd3a4cec8d782d2bdc205f3709827b776fd5c863a952fea97a14a6c2ee3f20432b8baa084470179078bd6a83597478b2fd9ae00ecb424822cb0d61e9a55a4",
            "b8565db06c2fa493e09b6764f4d09296422095eb6e9890f606654713bfee6f362a123688b61f254f315f18b20bcc5ed8b0b4f2224de9f498e3ef03532a8bcddb361f5ace8ff491bab8b3d06550496501264f9f48ebad277e7492146789d0fc1a3b1e3e81598370a4183683d1fee25a9a1fe359c836932746b983d01767ad4b9b3d70cc917fe57e41e0",
            true,
            CipherSuite::TLS_AEGIS_256_SHA512},
        CipherParams{
            "77b473865175ebd5ddf9c382bac227029c25bdb836e683a138e4618cc964488b",
            "f183d8de1e6dd4ccefa79fe22fabfda58e68dd29116d13408042f0713a4ee5f8",
            0,
            "0679fd74a846965e33e558676115d843e440fa37092fbd5c57c82fd914210fcf948f911b04632d66be46248d772b3eb9f55b537e54b1ec751b63f035c8",
            "9888b8ee03c3217a777b7558a31e331909570ea196f02c8cffad2c8dc6499b8125363c06a71c057842666bfb5c6acc937d2eecd960330c2361abdd88a4b191557ddf5102de75ddc7e09aee9862f32e24f1db3847a5f5b379fb32e2ef7ffb0d3a60",
            "3464d835302583ade6ed99e23333e865d3308f31a6cb65bcefdc9a1b9b4d0e0f75513188480dac4a64922af4441324ce7de74eb9f7f4e414f6177a4814edc96313694b99ff8dd36b2f7f79c7ecd70ec475abe1c1909238767f172fd6b95e92c025b1f8c9704d7b845964e14ccb333f0d4b",
            true,
            CipherSuite::TLS_AEGIS_256_SHA512},
        CipherParams{
            "b8c6e8cea59ca9fd2922530ee61911c1ed1c5af98be8fb03cbb449adcea0ed83",
            "af5bc1abe7bafadee790390277874cdfcc1ac1955f249d1131555d345832f555",
            0,
            "d899366a0b4e4d86cce5ba61aca2a84349c8de5757e008e94e7d7a3703",
            "b6c15f560be043d06aa27e15d8c901af6b19db7a15e1",
            "4c8496dfa6c419ef3c4867769a9014bd17118c22eef5f0f7ed5cb9ba59df21310c274cf9a585",
            true,
            CipherSuite::TLS_AEGIS_256_SHA512},
        CipherParams{
            "0000000000000000000000000000000000000000000000000000000000000000",
            "0000000000000000000000000000000000000000000000000000000000000000",
            0,
            "6dd5e43033fa6f021059a353edaf1f870387693054d0a2360fd1f6941a68f48ba972a1bc0816a446a6186e4a9a2f9df556bf709470137b8e60d9daa2",
            "dc5180954df0c3391a60b44cbf70aee72b7dbb2addc90a0bf2ceac6113287eb501fe1ea9f4c51822664b82fe0279b039f4",
            "c8a7d9131cebfa5388003cc30deac523aa9b09d148affff06ba40400e09ca900db770e07cedf5cd0647f6723c810ffcb596cac51edd6f49cd7be0010a3ac29e704",
            false,
            CipherSuite::TLS_AEGIS_256_SHA512}));
#endif
} // namespace test
} // namespace fizz
