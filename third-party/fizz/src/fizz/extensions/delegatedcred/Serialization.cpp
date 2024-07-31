#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>
#include <fizz/extensions/delegatedcred/Serialization.h>
#include <folly/Format.h>
#include <folly/Range.h>
#include <folly/base64.h>

namespace fizz {
namespace extensions {

namespace {
static constexpr folly::StringPiece kServerDCHeader =
    "-----BEGIN SERVER DELEGATED CREDENTIAL-----\n";
static constexpr folly::StringPiece kServerDCFooter =
    "-----END SERVER DELEGATED CREDENTIAL-----\n";
static constexpr folly::StringPiece kClientDCHeader =
    "-----BEGIN CLIENT DELEGATED CREDENTIAL-----\n";
static constexpr folly::StringPiece kClientDCFooter =
    "-----END CLIENT DELEGATED CREDENTIAL-----\n";
static constexpr folly::StringPiece kClientDCKeyHeader =
    "-----BEGIN CLIENT DC PRIVATE KEY-----\n";
static constexpr folly::StringPiece kClientDCKeyFooter =
    "-----END CLIENT DC PRIVATE KEY-----\n";
static constexpr folly::StringPiece kServerDCKeyHeader =
    "-----BEGIN SERVER DC PRIVATE KEY-----\n";
static constexpr folly::StringPiece kServerDCKeyFooter =
    "-----END SERVER DC PRIVATE KEY-----\n";
static constexpr folly::StringPiece kPrivateKeyHeader =
    "-----BEGIN PRIVATE KEY-----\n";
static constexpr folly::StringPiece kPrivateKeyFooter =
    "-----END PRIVATE KEY-----\n";
} // namespace

std::string generateDelegatedCredentialPEM(
    DelegatedCredentialMode mode,
    DelegatedCredential credential,
    std::string credKeyData) {
  std::string pemData;
  // Append credential
  auto encodedCred = fizz::extensions::encodeExtension(credential);
  pemData += mode == DelegatedCredentialMode::Client ? kClientDCHeader
                                                     : kServerDCHeader;
  pemData += folly::base64Encode(encodedCred.extension_data->to<std::string>());
  pemData += "\n";
  pemData += mode == DelegatedCredentialMode::Client ? kClientDCFooter
                                                     : kServerDCFooter;

  // Replace labels for key and append
  auto keyHeaderPtr = credKeyData.find(kPrivateKeyHeader);
  auto keyFooterPtr = credKeyData.find(kPrivateKeyFooter);
  if (keyHeaderPtr == std::string::npos || keyFooterPtr == std::string::npos) {
    throw std::runtime_error("Invalid key data");
  }

  auto header = mode == DelegatedCredentialMode::Client ? kClientDCKeyHeader
                                                        : kServerDCKeyHeader;
  auto footer = mode == DelegatedCredentialMode::Client ? kClientDCKeyFooter
                                                        : kServerDCKeyFooter;
  credKeyData.replace(keyFooterPtr, kPrivateKeyFooter.size(), footer);
  credKeyData.replace(0, kPrivateKeyHeader.size(), header);
  pemData += credKeyData;

  return pemData;
}

std::unique_ptr<SelfDelegatedCredential> loadDCFromPEM(
    std::string combinedPemData,
    DelegatedCredentialMode mode) {
  auto certs = folly::ssl::OpenSSLCertUtils::readCertsFromBuffer(
      folly::StringPiece(combinedPemData));

  auto dcKeyHeader = mode == DelegatedCredentialMode::Server
      ? kServerDCKeyHeader
      : kClientDCKeyHeader;
  auto dcKeyFooter = mode == DelegatedCredentialMode::Server
      ? kServerDCKeyFooter
      : kClientDCKeyFooter;
  auto keyHeaderPtr = combinedPemData.find(dcKeyHeader);
  auto keyFooderPtr = combinedPemData.find(dcKeyFooter);

  if (!(keyHeaderPtr != std::string::npos &&
        keyFooderPtr != std::string::npos)) {
    throw std::runtime_error(folly::sformat(
        "Failed to load delegated credential key from pem, expected label {} which was not found",
        dcKeyHeader));
  }
  // Replace out custom dc key labels with the standard private key labels so
  // the key can be read as normal
  auto keyBuffer = combinedPemData.substr(
      keyHeaderPtr, keyFooderPtr - keyHeaderPtr + dcKeyFooter.size());

  keyBuffer.replace(0, dcKeyHeader.size(), kPrivateKeyHeader);
  keyBuffer.replace(
      keyBuffer.size() - dcKeyFooter.size(),
      dcKeyFooter.size(),
      kPrivateKeyFooter);

  auto privKey = fizz::openssl::CertUtils::readPrivateKeyFromBuffer(keyBuffer);

  auto dcHeader = mode == DelegatedCredentialMode::Server ? kServerDCHeader
                                                          : kClientDCHeader;
  auto dcFooter = mode == DelegatedCredentialMode::Server ? kServerDCFooter
                                                          : kClientDCFooter;

  auto credDataPtr = combinedPemData.find(dcHeader);
  auto credDataEndPtr = combinedPemData.find(dcFooter);
  if (!(credDataPtr != std::string::npos &&
        credDataEndPtr != std::string::npos)) {
    throw std::runtime_error(folly::sformat(
        "Failed to load delegated credential from pem, expected label {} which was not found",
        dcHeader));
  }
  folly::Optional<DelegatedCredential> cred;
  try {
    auto credData = folly::base64Decode(combinedPemData.substr(
        credDataPtr + dcHeader.size(),
        credDataEndPtr - credDataPtr - dcHeader.size() - 1));

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
      mode, std::move(certs), std::move(privKey), std::move(*cred));
}
} // namespace extensions
} // namespace fizz
