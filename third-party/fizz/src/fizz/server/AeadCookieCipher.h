/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <boost/variant.hpp>

#include <fizz/server/AeadTokenCipher.h>
#include <fizz/server/CookieCipher.h>
#include <fizz/server/FizzServerContext.h>

namespace fizz {
namespace server {

struct AppToken {
  Buf token;
};

struct StatelessHelloRetryRequest {
  Buf data;
};

class AeadCookieCipher : public CookieCipher {
 public:
  AeadCookieCipher()
      : tokenCipher_(std::vector<std::string>({"Fizz Cookie Cipher v1"})) {}

  /**
   * Set cookie secrets to use for cookie encryption/decryption.
   */
  bool setCookieSecrets(const std::vector<folly::ByteRange>& cookieSecrets) {
    return tokenCipher_.setSecrets(cookieSecrets);
  }

  /**
   * Set the Fizz context to use when negotiating the parameters for a stateless
   * hello retry request.
   */
  void setContext(const FizzServerContext* context) {
    context_ = context;
  }

  /**
   * Returns either a stateless hello retry request, or a verified token
   * contained in the client hello.
   */
  boost::variant<AppToken, StatelessHelloRetryRequest> getTokenOrRetry(
      Buf clientHello,
      Buf appToken) const;

  folly::Optional<CookieState> decrypt(Buf cookie) const override;

 private:
  Buf getStatelessResponse(const ClientHello& chlo, Buf appToken) const;

  Aead128GCMTokenCipher tokenCipher_;

  const FizzServerContext* context_ = nullptr;
};
} // namespace server
} // namespace fizz
