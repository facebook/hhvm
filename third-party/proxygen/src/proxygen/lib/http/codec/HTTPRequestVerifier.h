/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/HeaderConstants.h>
#include <proxygen/lib/http/codec/CodecUtil.h>

namespace proxygen {

class HTTPRequestVerifier {
 public:
  explicit HTTPRequestVerifier() {
  }

  void reset(HTTPMessage* msg) {
    msg_ = msg;
    error = "";
    hasMethod_ = false;
    hasPath_ = false;
    hasScheme_ = false;
    hasAuthority_ = false;
    hasUpgradeProtocol_ = false;
  }

  bool setMethod(folly::StringPiece method) {
    if (hasMethod_) {
      error = "Duplicate method";
      return false;
    }
    if (!CodecUtil::validateMethod(method)) {
      error = folly::to<std::string>("Invalid method: ", method);
      return false;
    }
    hasMethod_ = true;
    assert(msg_ != nullptr);
    msg_->setMethod(method);
    return true;
  }

  bool setPath(folly::StringPiece path,
               bool strictValidation,
               bool allowEmptyPath) {
    if (hasPath_) {
      error = "Duplicate path";
      return false;
    }
    if (!CodecUtil::validateURL(path,
                                strictValidation
                                    ? URLValidateMode::STRICT
                                    : URLValidateMode::STRICT_COMPAT)) {
      error = folly::to<std::string>("Invalid url: ", path);
      return false;
    }
    hasPath_ = true;
    assert(msg_ != nullptr);
    // Relax strictValidation here if empty paths are allowed and it's empty
    strictValidation &= !(allowEmptyPath && path.empty());
    auto parseUrl = msg_->setURL(path.str(), strictValidation);
    if (strictValidation && !parseUrl.valid()) {
      error = folly::to<std::string>("Invalid url: ", path);
      return false;
    }
    return !strictValidation || parseUrl.valid();
  }

  bool setScheme(folly::StringPiece scheme) {
    if (hasScheme_) {
      error = "Duplicate scheme";
      return false;
    }
    // This just checks for alpha chars
    if (!CodecUtil::validateScheme(scheme)) {
      error = folly::to<std::string>("Invalid scheme: ", scheme);
      return false;
    }
    hasScheme_ = true;
    // TODO support non http/https schemes
    if (scheme == headers::kHttps) {
      assert(msg_ != nullptr);
      msg_->setSecure(true);
    } else if (scheme == headers::kMasque) {
      assert(msg_ != nullptr);
      msg_->setMasque();
    }
    return true;
  }

  bool setAuthority(folly::StringPiece authority,
                    bool validate,
                    bool strictValidation) {
    if (hasAuthority_) {
      error = "Duplicate authority";
      return false;
    }
    if (validate &&
        !CodecUtil::validateHeaderValue(
            authority,
            strictValidation ? CodecUtil::CtlEscapeMode::STRICT
                             : CodecUtil::CtlEscapeMode::STRICT_COMPAT)) {
      error = folly::to<std::string>("Invalid authority: ", authority);
      return false;
    }
    hasAuthority_ = true;
    assert(msg_ != nullptr);
    msg_->getHeaders().add(HTTP_HEADER_HOST, authority);
    return true;
  }

  bool setUpgradeProtocol(folly::StringPiece protocol, bool strictValidation) {
    if (hasUpgradeProtocol_) {
      error = "Duplicate protocol";
      return false;
    }
    if (strictValidation && !CodecUtil::validateHeaderValue(
                                protocol, CodecUtil::CtlEscapeMode::STRICT)) {
      error = folly::to<std::string>("Invalid protocol: ", protocol);
      return false;
    }
    setHasUpgradeProtocol(true);
    msg_->setUpgradeProtocol(folly::to<std::string>(protocol));
    return true;
  }

  bool validate() {
    if (error.size()) {
      return false;
    }
    if (msg_->getMethod() == HTTPMethod::CONNECT) {
      if ((!hasUpgradeProtocol_ &&
           (!hasMethod_ || !hasAuthority_ || hasScheme_ || hasPath_)) ||
          (hasUpgradeProtocol_ && (!hasScheme_ || !hasPath_))) {
        error = folly::to<std::string>("Malformed CONNECT request m/a/s/pa/pr=",
                                       hasMethod_,
                                       hasAuthority_,
                                       hasScheme_,
                                       hasPath_,
                                       hasUpgradeProtocol_);
      }
    } else if (hasUpgradeProtocol_ || !hasMethod_ || !hasScheme_ || !hasPath_) {
      error = folly::to<std::string>("Malformed request m/a/s/pa/pr=",
                                     hasMethod_,
                                     hasAuthority_,
                                     hasScheme_,
                                     hasPath_,
                                     hasUpgradeProtocol_);
    }
    return error.empty();
  }

  void setMessage(HTTPMessage* msg) {
    msg_ = msg;
  }

  void setHasMethod(bool hasMethod) {
    hasMethod_ = hasMethod;
  }

  void setHasPath(bool hasPath) {
    hasPath_ = hasPath;
  }

  void setHasScheme(bool hasScheme) {
    hasScheme_ = hasScheme;
  }

  void setHasAuthority(bool hasAuthority) {
    hasAuthority_ = hasAuthority;
  }

  void setHasUpgradeProtocol(bool val) {
    hasUpgradeProtocol_ = val;
  }

  bool hasUpgradeProtocol() {
    return hasUpgradeProtocol_;
  }

  std::string error;

 private:
  HTTPMessage* msg_{nullptr};
  bool hasMethod_{false};
  bool hasPath_{false};
  bool hasScheme_{false};
  bool hasAuthority_{false};
  bool hasUpgradeProtocol_{false};
};

} // namespace proxygen
