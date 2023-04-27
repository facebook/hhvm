/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/ECCurve.h>
#include <fizz/crypto/signature/Signature.h>
#include <folly/String.h>

using namespace folly;
using namespace folly::ssl;

namespace fizz {
namespace testing {

struct Params {
  std::string msg;
  std::string priv; // x
  std::string pubX; // Ux
  std::string pubY; // Uy
};

class ECDSATest : public ::testing::TestWithParam<Params> {
  void SetUp() override {
    OpenSSL_add_all_algorithms();
  }
};

class ECDSA256Test : public ECDSATest {};
class ECDSA384Test : public ECDSATest {};
class ECDSA521Test : public ECDSATest {};

void setPoint(EcKeyUniquePtr& key, std::string x, std::string y) {
  auto binX = unhexlify(x);
  auto binY = unhexlify(y);
  BIGNUMUniquePtr numX(BN_bin2bn((uint8_t*)binX.data(), binX.size(), nullptr));
  BIGNUMUniquePtr numY(BN_bin2bn((uint8_t*)binY.data(), binY.size(), nullptr));
  EC_KEY_set_public_key_affine_coordinates(key.get(), numX.get(), numY.get());
}

EvpPkeyUniquePtr getKey(int nid, const Params& param) {
  auto privKeyBin = unhexlify(param.priv);
  BIGNUMUniquePtr privateBn(
      BN_bin2bn((uint8_t*)privKeyBin.c_str(), privKeyBin.size(), nullptr));
  EcKeyUniquePtr privateKey(EC_KEY_new_by_curve_name(nid));
  EC_KEY_set_private_key(privateKey.get(), privateBn.get());
  setPoint(privateKey, param.pubX, param.pubY);
  EvpPkeyUniquePtr pkeyPrivateKey(EVP_PKEY_new());
  EVP_PKEY_set1_EC_KEY(pkeyPrivateKey.get(), privateKey.get());
  return pkeyPrivateKey;
}

void modifySig(folly::IOBuf* sig) {
  auto& sigPtr = sig->writableData()[10];
  if (sigPtr == 1) {
    sigPtr = 2;
  } else {
    sigPtr = 1;
  }
}

void modifyData(folly::IOBuf* sig, std::string& msg) {
  auto& sigPtr = sig->writableData()[10];
  if (sigPtr == 1) {
    sigPtr = 2;
  } else {
    sigPtr = 1;
  }
  auto& msgPtr = msg[2];
  if (msgPtr == 1) {
    msgPtr = 2;
  } else {
    msgPtr = 1;
  }
}

TEST_P(ECDSA256Test, TestSignature) {
  auto key = getKey(P256::curveNid, GetParam());
  OpenSSLSignature<KeyType::P256> ecdsa;
  ecdsa.setKey(std::move(key));
  {
    std::string msg = GetParam().msg;
    auto sig = ecdsa.sign<SignatureScheme::ecdsa_secp256r1_sha256>(
        IOBuf::copyBuffer(msg)->coalesce());
    ecdsa.verify<SignatureScheme::ecdsa_secp256r1_sha256>(
        IOBuf::copyBuffer(msg)->coalesce(), sig->coalesce());
  }
  {
    std::string msg = GetParam().msg;
    auto sig = ecdsa.sign<SignatureScheme::ecdsa_secp256r1_sha256>(
        IOBuf::copyBuffer(msg)->coalesce());
    modifySig(sig.get());
    EXPECT_THROW(
        ecdsa.verify<SignatureScheme::ecdsa_secp256r1_sha256>(
            IOBuf::copyBuffer(msg)->coalesce(), sig->coalesce()),
        std::runtime_error);
  }
  {
    std::string msg = GetParam().msg;
    auto sig = ecdsa.sign<SignatureScheme::ecdsa_secp256r1_sha256>(
        IOBuf::copyBuffer(msg)->coalesce());
    modifyData(sig.get(), msg);
    EXPECT_THROW(
        ecdsa.verify<SignatureScheme::ecdsa_secp256r1_sha256>(
            IOBuf::copyBuffer(msg)->coalesce(), sig->coalesce()),
        std::runtime_error);
  }
}

TEST_P(ECDSA384Test, TestSignature) {
  auto key = getKey(P384::curveNid, GetParam());
  OpenSSLSignature<KeyType::P384> ecdsa;
  ecdsa.setKey(std::move(key));
  {
    std::string msg = GetParam().msg;
    auto sig = ecdsa.sign<SignatureScheme::ecdsa_secp384r1_sha384>(
        IOBuf::copyBuffer(msg)->coalesce());
    ecdsa.verify<SignatureScheme::ecdsa_secp384r1_sha384>(
        IOBuf::copyBuffer(msg)->coalesce(), sig->coalesce());
  }
  {
    std::string msg = GetParam().msg;
    auto sig = ecdsa.sign<SignatureScheme::ecdsa_secp384r1_sha384>(
        IOBuf::copyBuffer(msg)->coalesce());
    modifySig(sig.get());
    EXPECT_THROW(
        ecdsa.verify<SignatureScheme::ecdsa_secp384r1_sha384>(
            IOBuf::copyBuffer(msg)->coalesce(), sig->coalesce()),
        std::runtime_error);
  }
  {
    std::string msg = GetParam().msg;
    auto sig = ecdsa.sign<SignatureScheme::ecdsa_secp384r1_sha384>(
        IOBuf::copyBuffer(msg)->coalesce());
    modifyData(sig.get(), msg);
    EXPECT_THROW(
        ecdsa.verify<SignatureScheme::ecdsa_secp384r1_sha384>(
            IOBuf::copyBuffer(msg)->coalesce(), sig->coalesce()),
        std::runtime_error);
  }
}

TEST_P(ECDSA521Test, TestSignature) {
  auto key = getKey(P521::curveNid, GetParam());
  OpenSSLSignature<KeyType::P521> ecdsa;
  ecdsa.setKey(std::move(key));
  {
    std::string msg = GetParam().msg;
    auto sig = ecdsa.sign<SignatureScheme::ecdsa_secp521r1_sha512>(
        IOBuf::copyBuffer(msg)->coalesce());
    ecdsa.verify<SignatureScheme::ecdsa_secp521r1_sha512>(
        IOBuf::copyBuffer(msg)->coalesce(), sig->coalesce());
  }
  {
    std::string msg = GetParam().msg;
    auto sig = ecdsa.sign<SignatureScheme::ecdsa_secp521r1_sha512>(
        IOBuf::copyBuffer(msg)->coalesce());
    modifySig(sig.get());
    EXPECT_THROW(
        ecdsa.verify<SignatureScheme::ecdsa_secp521r1_sha512>(
            IOBuf::copyBuffer(msg)->coalesce(), sig->coalesce()),
        std::runtime_error);
  }
  {
    std::string msg = GetParam().msg;
    auto sig = ecdsa.sign<SignatureScheme::ecdsa_secp521r1_sha512>(
        IOBuf::copyBuffer(msg)->coalesce());
    modifyData(sig.get(), msg);
    EXPECT_THROW(
        ecdsa.verify<SignatureScheme::ecdsa_secp521r1_sha512>(
            IOBuf::copyBuffer(msg)->coalesce(), sig->coalesce()),
        std::runtime_error);
  }
}

// Test vector from https://tools.ietf.org/html/rfc6979#appendix-A.2.5
// We can't test those directly since we'd need to use the more complicated
// API of actually setting k and dealing with ECDSA_sig objects directly.
INSTANTIATE_TEST_SUITE_P(
    TestVectors,
    ECDSA256Test,
    ::testing::Values(Params{
        "sample",
        "C9AFA9D845BA75166B5C215767B1D6934E50C3DB36E89B127B8A622B120F6721",
        "60FED4BA255A9D31C961EB74C6356D68C049B8923B61FA6CE669622E60F29FB6",
        "7903FE1008B8BC99A41AE9E95628BC64F2F1B20C2D7E9F5177A3C294D4462299"}));
INSTANTIATE_TEST_SUITE_P(
    TestVectors,
    ECDSA384Test,
    ::testing::Values(Params{
        "sample",
        "6B9D3DAD2E1B8C1C05B19875B6659F4DE23C3B667BF297BA9AA47740787137D8"
        "96D5724E4C70A825F872C9EA60D2EDF5",
        "EC3A4E415B4E19A4568618029F427FA5DA9A8BC4AE92E02E06AAE5286B300C64"
        "DEF8F0EA9055866064A254515480BC13",
        "8015D9B72D7D57244EA8EF9AC0C621896708A59367F9DFB9F54CA84B3F1C9DB1"
        "288B231C3AE0D4FE7344FD2533264720"}));
INSTANTIATE_TEST_SUITE_P(
    TestVectors,
    ECDSA521Test,
    ::testing::Values(Params{
        "sample",
        // NOTE these are 0 padded at the beginning for unhexlify
        "00FAD06DAA62BA3B25D2FB40133DA757205DE67F5BB0018FEE8C86E1B68C7E75C"
        "AA896EB32F1F47C70855836A6D16FCC1466F6D8FBEC67DB89EC0C08B0E996B83"
        "538",
        "01894550D0785932E00EAA23B694F213F8C3121F86DC97A04E5A7167DB4E5BCD3"
        "71123D46E45DB6B5D5370A7F20FB633155D38FFA16D2BD761DCAC474B9A2F502"
        "3A4",
        "00493101C962CD4D2FDDF782285E64584139C2F91B47F87FF82354D6630F746A2"
        "8A0DB25741B5B34A828008B22ACC23F924FAAFBD4D33F81EA66956DFEAA2BFDF"
        "CF5"}));
} // namespace testing
} // namespace fizz
