#pragma once

#include <fizz/fizz-config.h>

#if FIZZ_CERTIFICATE_USE_OPENSSL_CERT
#include <folly/io/async/AsyncTransportCertificate.h>

namespace fizz {
using Cert = folly::AsyncTransportCertificate;
}

#else
#include <optional>
#include <string>

namespace fizz {

class Cert {
 public:
  virtual ~Cert() = default;
  virtual std::string getIdentity() const = 0;
  virtual std::optional<std::string> getDER() const = 0;
};
} // namespace fizz
#endif
