/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/compression/BrotliCertificateCompressor.h>
#include <fizz/compression/BrotliCertificateDecompressor.h>
#include <fizz/crypto/Utils.h>
#include <fizz/protocol/test/Matchers.h>
#include <fizz/protocol/test/Utilities.h>
#include <fizz/record/Extensions.h>

using namespace folly;
using namespace testing;

namespace fizz {
namespace test {

static const std::string exampleCompressedCertificate =
    "000200017b0001241b7a010064d2bf370fbf02930a9c446d30b65e274853274a"
    "2707a0298469da5ad6020b24d604db012ca81bf665b5a1383800588a653cccc7"
    "a205e36c27700cc73100e9a868e48aad36f5490287c6881ed25211ee24153bee"
    "ee8ab82167c0e818980ec4ef10881772271bc1c0121c969eaec8258abc3b14e2"
    "6644da5005aaaba8200d350d1fa72a70bc4d7943762aeab915c700be20306ac2"
    "0490161d7c1b7a15c6fc6539ba6ee53d791d2cbb55b4cb810f909fa367616ef8"
    "c62d75b172f09ffadef213f8f3dad67addd9f87909b61b3a38a8d5f9c4f95785"
    "692fe48ab004d00c171138e29ccfcd9e0a56c95d77f9f978b5bbbd9041416a63"
    "57f2c74e56012d5d895500944bb54429e86c72127d3e3f73d3fdfaaf3b96f115"
    "9ff7d0cc2dbb9e7853120400";

static const std::string tooLargeCompressedCertificate =
    "000205e17c0001295b7be10540a6f4a8dd2427787382a607e80e4005773fef06"
    "07a02984eb3aac830db2f2b6f000b6380ddb608ee374145607281ee3008c4419"
    "30546b1c6d3806310c02444b492d9b63b9a747e0183222e9100d25ee4650b261"
    "6e2e2417e2b0d1d2331e88efe0240fe20a134ecfece39f90a0c0250abc3312e4"
    "6220b590325253564188d4f0362aa3f231e185d828a9f6560c025c81432adc18"
    "100fd42f3b4dc1b726edfb47719dbbac6d5cb4f17f31ee9baa4b35d3067e3cfa"
    "0ecbe54a37ff450d390fd6ae72cc14f08f453b350f6bb9babc9584d9c94cc358"
    "cb425ee95e17e096009961c22632cecdf29c7721ea22eb91af5a6de2fda3ef09"
    "74517bfb59f1814286d8273b2602d686f22609a2e7df6f5ae6567fdcd9753177"
    "b6ee5a8a9cb775b1e51c4fdae63000dd03";
class BrotliCertificateCompressorTest : public testing::Test {
 public:
  void SetUp() override {
    CryptoUtils::init();
    compressor_ = std::make_unique<BrotliCertificateCompressor>();
    decompressor_ = std::make_unique<BrotliCertificateDecompressor>();
  }

  void TearDown() override {}

 protected:
  template <class T>
  T decodeHex(const std::string& hex) {
    auto data = unhexlify(hex);
    auto buf = IOBuf::copyBuffer(data.data(), data.size());
    return decode<T>(std::move(buf));
  }

  template <class T>
  std::string encodeHex(T&& msg) {
    auto buf = encode(std::forward<T>(msg));
    auto str = buf->moveToFbString().toStdString();
    return hexlify(str);
  }
  std::unique_ptr<BrotliCertificateCompressor> compressor_;
  std::unique_ptr<BrotliCertificateDecompressor> decompressor_;
};

TEST_F(BrotliCertificateCompressorTest, TestCompressDecompress) {
  auto certAndKey = createCert("fizz-selfsigned", false, nullptr);
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(std::move(certAndKey.cert));
  auto cert =
      CertUtils::makeSelfCert(std::move(certs), std::move(certAndKey.key));
  auto certMsg = cert->getCertMessage();

  // Add extension
  CertificateAuthorities auth;
  DistinguishedName dn;
  dn.encoded_name = IOBuf::copyBuffer("DistinguishedName");
  auth.authorities.push_back(std::move(dn));
  certMsg.certificate_list[0].extensions.push_back(encodeExtension(auth));

  auto compressedCertMsg = compressor_->compress(certMsg);
  EXPECT_EQ(
      compressedCertMsg.algorithm, CertificateCompressionAlgorithm::brotli);

  auto decompressedCertMsg = decompressor_->decompress(compressedCertMsg);
  EXPECT_TRUE(decompressedCertMsg.certificate_request_context->empty());
  EXPECT_EQ(decompressedCertMsg.certificate_list.size(), 1);
  auto& certEntry = decompressedCertMsg.certificate_list.at(0);
  EXPECT_EQ(certEntry.extensions.size(), 1);
  auto decompressedPeer = CertUtils::makePeerCert(certEntry.cert_data->clone());
  EXPECT_EQ(decompressedPeer->getIdentity(), cert->getIdentity());

  EXPECT_TRUE(IOBufEqualTo()(encode(certMsg), encode(decompressedCertMsg)));
}

TEST_F(BrotliCertificateCompressorTest, TestHugeCompressedCert) {
  auto cc = decodeHex<CompressedCertificate>(tooLargeCompressedCertificate);

  try {
    decompressor_->decompress(cc);
    FAIL() << "Decompressor decompressed excessively large cert";
  } catch (const std::exception& e) {
    EXPECT_THAT(
        e.what(), HasSubstr("exceeds maximum certificate message size"));
  }

  // Lie about size, should still error.
  cc.uncompressed_length = 64;

  try {
    decompressor_->decompress(cc);
    FAIL() << "Decompressor decompressed cert erroneously";
  } catch (const std::exception& e) {
    EXPECT_THAT(e.what(), HasSubstr("Decompressing certificate failed"));
  }
}

TEST_F(BrotliCertificateCompressorTest, TestCompressedLengthTooBig) {
  auto compressedCert =
      decodeHex<CompressedCertificate>(exampleCompressedCertificate);

  // Lie about having a larger cert.
  compressedCert.uncompressed_length += 1;
  try {
    decompressor_->decompress(compressedCert);
    FAIL() << "Decompressor decompressed cert erroneously";
  } catch (const std::exception& e) {
    EXPECT_THAT(e.what(), HasSubstr("Uncompressed length incorrect"));
  }
}

TEST_F(BrotliCertificateCompressorTest, TestCompressedCertTruncated) {
  auto compressedCert =
      decodeHex<CompressedCertificate>(exampleCompressedCertificate);

  // Truncate length
  compressedCert.uncompressed_length -= 1;
  try {
    decompressor_->decompress(compressedCert);
    FAIL() << "Decompressor decompressed cert erroneously";
  } catch (const std::exception& e) {
    EXPECT_THAT(e.what(), HasSubstr("Decompressing certificate failed"));
  }
}

TEST_F(BrotliCertificateCompressorTest, TestCompressedCertEmpty) {
  CompressedCertificate compressedCert;
  compressedCert.uncompressed_length = 0;
  compressedCert.algorithm = CertificateCompressionAlgorithm::brotli;
  compressedCert.compressed_certificate_message = IOBuf::create(0);
  try {
    decompressor_->decompress(compressedCert);
    FAIL() << "Decompressor decompressed cert erroneously";
  } catch (const std::exception& e) {
    EXPECT_THAT(e.what(), HasSubstr("Decompressing certificate failed"));
  }
}

TEST_F(BrotliCertificateCompressorTest, TestCompressedCertBadAlgo) {
  auto compressedCert =
      decodeHex<CompressedCertificate>(exampleCompressedCertificate);

  // Bad algorithm value
  compressedCert.algorithm =
      static_cast<CertificateCompressionAlgorithm>(0xdead);
  try {
    decompressor_->decompress(compressedCert);
    FAIL() << "Decompressor decompressed cert erroneously";
  } catch (const std::exception& e) {
    EXPECT_THAT(e.what(), HasSubstr("non-brotli algorithm"));
  }
}

} // namespace test
} // namespace fizz
