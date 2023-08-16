/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/compression/ZstdCertificateCompressor.h>
#include <fizz/compression/ZstdCertificateDecompressor.h>
#include <fizz/crypto/Utils.h>
#include <fizz/protocol/test/Matchers.h>
#include <fizz/protocol/test/Utilities.h>
#include <fizz/record/Extensions.h>

using namespace folly;
using namespace testing;

namespace fizz {
namespace test {

static const std::string exampleCompressedCertificate =
    "000300042c0003d128b52ffd602c033d1e00643a000004280004233082041f30"
    "82020702090099ff635fbc116e8f300d06092a864886f70d01010b0500305331"
    "0b300906035504061302555331153013070c0c44656661756c74204369747931"
    "1c301a0a0c136f6d70616e79204c7464310f300d030c06666f6f626172301e17"
    "0d3138303430363232303031365a393038313930500c300a0362617a30820122"
    "0105000382010f000a0282010100b5c119f5d7d9454cd5c353631712b234701d"
    "f3c13035a700f59ea83c077a8721448a37a289fd8e5edce3683db926abf51eb6"
    "9d31443d3025e1e31f30867ec5190a09b60f97dd60e6206e903b4d6afa137da2"
    "eb765a6a2f5196334611482040af243a66ed823a3acee193f1f279295033b07e"
    "a22e714c5f64f5d7e9ec81565676e78c18065cddc7e863f654869db0c7a7c116"
    "1a180275d94181b9a28572d0e170d4548267e5b37812bcc69101221998765bba"
    "65266cbaf43923dea7f0870909675275ac21efe7bd99c5979aff526aedbe4829"
    "2b295f905822803f07f02bbd260b5af09b99cf3397f5ab21e2fbb2a0d2fccd39"
    "bd43d73b26118a5386ea3db6885f020301000103820201003592539ab5a6bc6b"
    "8dda8ad6e31bb444422546d12ae3bc77008e79086f6890d3493b2f8d89733e02"
    "35ae0f10e54d13967e2703ae76046d2a3151259b4a7e3bc9a0c59563903dfd1f"
    "bce3b8ba9ff5a98fef07b6e882147cf9a572c246603e2a7f70714b4761716dce"
    "061e1e05a4cd8d37f1c110784e79ff7e91220ef3d91b3c519e872f55d87e34c8"
    "6f046c157ed9c26426f1fed9141c60619b169c9131d890c0527aebe6fd73f4d8"
    "067bb50b3f141894ce48889564a0fe673f3e4774ae3e64b5a0323f8dd3b25ed2"
    "f4e1c4bef945f38bb05d8f4887ee0173184aef5b89b73b7c9c679d90c0292c88"
    "d94fca90cabef9da2d011d93d5b7a32c76b79bead42c54851e4636b27689d4de"
    "fd815a42f286fb146c54455a1c9dcef28eef7de80bc5450696105f7b1b3b7d66"
    "1be1ab6200751fe340484758af760bed96cfbd64cfa3c6038ac1173e3460469b"
    "2bbb4c373c3235675cd1f5bcf3e60feed6be514303850c8d9f6759520a20e47c"
    "9c24ff6fdd583f5a47d2cf906f2a74397b6cb4d2488921404d27ffdec5646c69"
    "aa28fa216744884a541f2b4a556a92574c5b50dffd2010cfde1f6f8fe5d1fb9e"
    "146b690ce0b3533ea23f3afdc3fb8d0ef77e5916f92ba22892a75d3d5175dd0b"
    "7a15226ab28a153ee6cafa84aa86b887e9ebec8a8f9f2c8ca06b66fd84befef6"
    "99f98f1a3be757385084852d2dd5330b7d8602976dda8bb000000b000bffab9b"
    "3b00ef3ece611880078481ab9509635041811a75466008e61e";

static const std::string tooLargeCompressedCertificate =
    "000310000000006b28b52ffd04585400001000000100fbff39c0024c00000800"
    "0100fcff3910024c000008000100fcff3910024c000008000100fcff3910024c"
    "000008000100fcff3910024c000008000100fcff3910024c000008000100fcff"
    "3910024d000008000100fcff391002f13e16e1";

class ZstdCertificateCompressorTest : public testing::Test {
 public:
  void SetUp() override {
    CryptoUtils::init();
    compressor_ = std::make_unique<ZstdCertificateCompressor>(19);
    decompressor_ = std::make_unique<ZstdCertificateDecompressor>();
  }

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
  std::unique_ptr<ZstdCertificateCompressor> compressor_;
  std::unique_ptr<ZstdCertificateDecompressor> decompressor_;
};

TEST_F(ZstdCertificateCompressorTest, TestCompressDecompress) {
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
  EXPECT_EQ(compressedCertMsg.algorithm, CertificateCompressionAlgorithm::zstd);

  auto decompressedCertMsg = decompressor_->decompress(compressedCertMsg);
  EXPECT_TRUE(decompressedCertMsg.certificate_request_context->empty());
  EXPECT_EQ(decompressedCertMsg.certificate_list.size(), 1);
  auto& certEntry = decompressedCertMsg.certificate_list.at(0);
  EXPECT_EQ(certEntry.extensions.size(), 1);
  auto decompressedPeer = CertUtils::makePeerCert(certEntry.cert_data->clone());
  EXPECT_EQ(decompressedPeer->getIdentity(), cert->getIdentity());

  EXPECT_TRUE(IOBufEqualTo()(encode(certMsg), encode(decompressedCertMsg)));
}

TEST_F(ZstdCertificateCompressorTest, TestCompressedCertEmpty) {
  CompressedCertificate compressedCert;
  compressedCert.uncompressed_length = 0;
  compressedCert.algorithm = CertificateCompressionAlgorithm::zstd;
  compressedCert.compressed_certificate_message = IOBuf::create(0);
  try {
    decompressor_->decompress(compressedCert);
    FAIL() << "Decompressor decompressed cert erroneously";
  } catch (const std::exception& e) {
    EXPECT_THAT(e.what(), HasSubstr("Compressed certificate is zero-length"));
  }
}

TEST_F(ZstdCertificateCompressorTest, TestHugeCompressedCert) {
  auto cc = decodeHex<CompressedCertificate>(tooLargeCompressedCertificate);

  try {
    decompressor_->decompress(cc);
    FAIL() << "Decompressor decompressed excessively large cert";
  } catch (const std::exception& e) {
    EXPECT_THAT(
        e.what(), HasSubstr("exceeds maximum certificate message size"));
  }
}

TEST_F(ZstdCertificateCompressorTest, TestHugeCompressedCertFakeSize) {
  auto cc = decodeHex<CompressedCertificate>(tooLargeCompressedCertificate);
  // Lie about size, should still error.
  cc.uncompressed_length = 64;

  try {
    decompressor_->decompress(cc);
    FAIL() << "Decompressor decompressed cert erroneously";
  } catch (const std::exception& e) {
    EXPECT_THAT(e.what(), HasSubstr("Destination buffer is too small"));
  }
}

TEST_F(ZstdCertificateCompressorTest, TestBadMessageTooLong) {
  auto compressedCert =
      decodeHex<CompressedCertificate>(exampleCompressedCertificate);
  auto actual = compressedCert.uncompressed_length;

  // Lie about having a larger cert.
  compressedCert.uncompressed_length = actual + 1;
  try {
    decompressor_->decompress(compressedCert);
    FAIL() << "Decompressor decompressed cert erroneously";
  } catch (const std::exception& e) {
    EXPECT_THAT(e.what(), HasSubstr("Uncompressed length incorrect"));
  }
}

TEST_F(ZstdCertificateCompressorTest, TestBadMessageTooShort) {
  auto compressedCert =
      decodeHex<CompressedCertificate>(exampleCompressedCertificate);
  auto actual = compressedCert.uncompressed_length;

  // Truncate length
  compressedCert.uncompressed_length = actual - 1;
  try {
    decompressor_->decompress(compressedCert);
    FAIL() << "Decompressor decompressed cert erroneously";
  } catch (const std::exception& e) {
    EXPECT_THAT(e.what(), HasSubstr("Uncompressed length incorrect"));
  }
}

TEST_F(ZstdCertificateCompressorTest, TestBadMessageWrongAlgo) {
  auto compressedCert =
      decodeHex<CompressedCertificate>(exampleCompressedCertificate);
  auto actual = compressedCert.uncompressed_length;

  // Bad algorithm value
  compressedCert.uncompressed_length = actual;
  compressedCert.algorithm = (CertificateCompressionAlgorithm)0xdead;
  try {
    decompressor_->decompress(compressedCert);
    FAIL() << "Decompressor decompressed cert erroneously";
  } catch (const std::exception& e) {
    EXPECT_THAT(e.what(), HasSubstr("non-zstd algorithm"));
  }
}

} // namespace test
} // namespace fizz
