#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>
#include <fizz/extensions/delegatedcred/Serialization.h>
#include <folly/Format.h>
#include <folly/Range.h>
#include <folly/base64.h>

namespace fizz {
namespace extensions {

namespace {
static constexpr folly::StringPiece kDCHeader =
    "-----BEGIN DELEGATED CREDENTIAL-----\n";
static constexpr folly::StringPiece kDCFooter =
    "-----END DELEGATED CREDENTIAL-----\n";
} // namespace

std::string generateDelegatedCredentialPEM(
    DelegatedCredential credential,
    std::string certData,
    std::string credKeyData) {
  auto encodedCred = fizz::extensions::encodeExtension(credential);
  std::string pemData;
  pemData += kDCHeader;
  pemData += folly::base64Encode(encodedCred.extension_data->to<std::string>());
  pemData += "\n";
  pemData += kDCFooter;
  pemData += credKeyData;
  pemData += certData;
  return pemData;
}

std::unique_ptr<SelfDelegatedCredential> loadDCFromPEM(
    std::string combinedPemData) {
  auto certs = folly::ssl::OpenSSLCertUtils::readCertsFromBuffer(
      folly::StringPiece(combinedPemData));
  auto privKey =
      fizz::openssl::CertUtils::readPrivateKeyFromBuffer(combinedPemData);
  auto credDataPtr = combinedPemData.find(kDCHeader);
  auto credDataEndPtr = combinedPemData.find(kDCFooter);
  if (!(credDataPtr != std::string::npos &&
        credDataEndPtr != std::string::npos)) {
    throw std::runtime_error(folly::sformat(
        "Failed to load delegated credential from pem, expected label {} which was not found",
        kDCHeader));
  }
  folly::Optional<DelegatedCredential> cred;
  try {
    auto credData = folly::base64Decode(combinedPemData.substr(
        credDataPtr + kDCHeader.size(),
        credDataEndPtr - credDataPtr - kDCHeader.size() - 1));

    std::vector<Extension> credVec;
    credVec.emplace_back(Extension{
        ExtensionType::delegated_credential,
        folly::IOBuf::copyBuffer(std::move(credData))});
    cred = getExtension<DelegatedCredential>(std::move(credVec));
  } catch (const std::exception& e) {
    throw std::runtime_error(folly::sformat(
        "Failed to decode delegated credential with exception {}", e.what()));
  }
  // Note we currently only support P256 this will throw if there is a mismatch
  // in the delegated creds expected verification aglorithm
  return std::make_unique<
      SelfDelegatedCredentialImpl<fizz::openssl::KeyType::P256>>(
      std::move(certs), std::move(privKey), std::move(*cred));
}
} // namespace extensions
} // namespace fizz
