/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/client/HTTPClient.h"
#include "proxygen/lib/http/coro/transport/HTTPConnectAsyncTransport.h"
#include <fizz/protocol/DefaultCertificateVerifier.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/utils/URL.h>

#include <folly/String.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GFlags.h>

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace proxygen;
using namespace proxygen::coro;

DEFINE_bool(quic, false, "Connect using QUIC/H3");
DEFINE_string(ca_file,
              "/etc/pki/tls/cert.pem",
              "CA file for validating server certificates");
DEFINE_int32(timeout_ms, 3000, "default connect/read timeout");
DEFINE_string(proxy, "", "proxy address");
DEFINE_string(method, "GET", "HTTP method");
DEFINE_string(post_body,
              "",
              "path of file to send in the body of the POST request");
DEFINE_bool(tls_proxy, true, "Use TLS to connect to proxy");
DEFINE_string(client_cert_path, "", "client cert path");
DEFINE_string(client_key_path, "", "client key path");
DEFINE_bool(fizz, true, "proxy endpoint TLS use fizz");
DEFINE_bool(no_server_cert_verifier,
            false,
            "Do not verify server identity presented (makes the client work "
            "against self-signed certs)");

folly::coro::Task<void> getViaProxy(folly::EventBase* evb,
                                    const std::string& urlStr,
                                    HTTPSourceReader reader);

folly::coro::Task<void> getWithCoro(folly::EventBase* evb,
                                    const std::string& urlStr,
                                    HTTPSourceReader reader);

folly::coro::Task<void> postWithCoro(folly::EventBase* evb,
                                     const std::string& urlStr,
                                     HTTPSourceReader reader);

namespace {
// This is an insecure certificate verifier and is not meant to be
// used in production. Using it in production would mean that this will
// leave everyone insecure.
class InsecureVerifierDangerousDoNotUseInProduction
    : public fizz::CertificateVerifier {
 public:
  ~InsecureVerifierDangerousDoNotUseInProduction() override = default;

  [[nodiscard]] std::shared_ptr<const folly::AsyncTransportCertificate> verify(
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs)
      const override {
    return certs.front();
  }

  [[nodiscard]] std::vector<fizz::Extension> getCertificateRequestExtensions()
      const override {
    return std::vector<fizz::Extension>();
  }
};

const HTTPClient::RequestHeaderMap& getDefaultHeaders() {
  /*
   * From RFC7231:
   * A request without any Accept header field implies that the user agent will
   * accept any media type in response.
   *
   * However this is explicitly sent due to misbehaving servers that do not
   * conform to spec.
   */
  static HTTPClient::RequestHeaderMap headers{{"accept", "*/*"}};
  return headers;
}
} // namespace

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  ::gflags::ParseCommandLineFlags(&argc, &argv, false);

  if (argc < 2) {
    XLOG(ERR) << "Usage: cocurl <url>";
    return 1;
  }

  folly::EventBase evb;
  int ret = 0;
  HTTPSourceReader reader;
  reader
      .onHeaders([](std::unique_ptr<HTTPMessage> response, bool, bool) {
        std::cout << "Received Headers:" << std::endl;
        response->describe(std::cout);
        return HTTPSourceReader::Continue;
      })
      .onPushPromise(
          [](std::unique_ptr<HTTPMessage> promise, HTTPSourceHolder, bool) {
            std::cout << "Received Push:" << std::endl;
            promise->describe(std::cout);
            return HTTPSourceReader::Continue;
          })
      .onBody([](BufQueue body, bool) {
        if (!body.empty()) {
          std::cout << folly::StringPiece(body.move()->coalesce());
        }
        return HTTPSourceReader::Continue;
      })
      .onTrailers([](std::unique_ptr<HTTPHeaders> trailers) {
        std::cout << "Received Headers:" << std::endl;
        trailers->forEach([](const std::string& h, const std::string& v) {
          std::cout << " " << stripCntrlChars(h) << ": " << stripCntrlChars(v)
                    << std::endl;
        });
      })
      .onError([&ret](HTTPSourceReader::ErrorContext ec, HTTPError error) {
        if (ec == HTTPSourceReader::ErrorContext::HEADERS) {
          std::cerr << "Error receiving response headers err="
                    << error.describe() << std::endl;
        } else {
          std::cerr << "Error receiving response body err=" << error.describe()
                    << std::endl;
        }
        ret = 1;
      });

  HTTPClient::setDefaultCAPaths({FLAGS_ca_file});

  if (FLAGS_no_server_cert_verifier) {
    auto verifier =
        std::make_shared<InsecureVerifierDangerousDoNotUseInProduction>();
    HTTPClient::setDefaultFizzCertVerifier(verifier);
  }
  std::string urlStr(argv[1]);

  folly::SemiFuture<folly::Unit> getFuture;

  std::string method = FLAGS_method;
  folly::toLowerAscii(method);

  if (!FLAGS_proxy.empty()) {
    getFuture =
        co_withExecutor(&evb, getViaProxy(&evb, urlStr, std::move(reader)))
            .start();
  } else if (method == "post") {
    getFuture =
        co_withExecutor(&evb, postWithCoro(&evb, urlStr, std::move(reader)))
            .start();
  } else {
    getFuture =
        co_withExecutor(&evb, getWithCoro(&evb, urlStr, std::move(reader)))
            .start();
  }

  std::move(getFuture).via(&evb).thenTry([&ret](folly::Try<void> getResult) {
    if (getResult.hasException()) {
      std::cerr << "Failed, err=" << getResult.exception() << std::endl;
      ret = 1;
    }
  });
  evb.loop();
  return ret;
}

folly::coro::Task<void> getWithCoro(folly::EventBase* evb,
                                    const std::string& urlStr,
                                    HTTPSourceReader reader) {
  proxygen::URL url(urlStr);
  auto defaultTimeout = std::chrono::milliseconds(FLAGS_timeout_ms);
  auto session =
      co_await HTTPClient::getHTTPSession(evb,
                                          url.getHost(),
                                          /*port*/ url.getPort(),
                                          /*isSecure*/ url.isSecure(),
                                          /*useQuic*/ FLAGS_quic,
                                          defaultTimeout,
                                          defaultTimeout,
                                          FLAGS_client_cert_path,
                                          FLAGS_client_key_path);

  XLOG(INFO) << "Sending GET for " << urlStr;
  co_await HTTPClient::get(session,
                           std::move(url),
                           std::move(reader),
                           defaultTimeout,
                           getDefaultHeaders());

  session->initiateDrain();
}

folly::coro::Task<void> postWithCoro(folly::EventBase* evb,
                                     const std::string& urlStr,
                                     HTTPSourceReader reader) {
  if (FLAGS_post_body.empty()) {
    throw std::runtime_error("missing --post_body <file>");
  }

  std::ifstream file(FLAGS_post_body);

  if (!file) {
    throw std::runtime_error("file could not be opened");
  }

  std::ostringstream ss;
  ss << file.rdbuf();

  proxygen::URL url(urlStr);
  auto defaultTimeout = std::chrono::milliseconds(FLAGS_timeout_ms);
  auto session =
      co_await HTTPClient::getHTTPSession(evb,
                                          url.getHost(),
                                          /*port*/ url.getPort(),
                                          /*isSecure*/ url.isSecure(),
                                          /*useQuic*/ FLAGS_quic,
                                          defaultTimeout,
                                          defaultTimeout,
                                          FLAGS_client_cert_path,
                                          FLAGS_client_key_path);

  XLOG(INFO) << "Sending POST for " << urlStr;
  co_await HTTPClient::post(session,
                            std::move(url),
                            ss.str(),
                            std::move(reader),
                            defaultTimeout,
                            getDefaultHeaders());

  session->initiateDrain();
}

folly::coro::Task<void> getViaProxy(folly::EventBase* evb,
                                    const std::string& urlStr,
                                    HTTPSourceReader reader) {
  folly::SocketAddress proxyAddr;
  proxyAddr.setFromHostPort(FLAGS_proxy);
  XLOG(INFO) << "Establishing HTTP connection to proxy: "
             << proxyAddr.describe();
  std::chrono::milliseconds timeout(FLAGS_timeout_ms);
  auto proxySession =
      co_await HTTPClient::getHTTPSession(evb,
                                          proxyAddr.getAddressStr(),
                                          proxyAddr.getPort(),
                                          FLAGS_tls_proxy,
                                          /*useQuic=*/false,
                                          timeout,
                                          timeout,
                                          FLAGS_client_cert_path,
                                          FLAGS_client_key_path);
  XLOG(INFO) << "Got HTTP connection to proxy: " << proxyAddr.describe();

  URL url(urlStr);
  if (!url.isValid() || !url.hasHost()) {
    throw std::runtime_error(folly::to<std::string>("Invalid URL: ", urlStr));
  }
  auto endpointSession = co_await HTTPClient::getHTTPSessionViaProxy(
      proxySession,
      url.getHost(),
      url.getPort(),
      /*connectUnique=*/true,
      url.isSecure() ? (FLAGS_fizz ? HTTPClient::SecureTransportImpl::FIZZ
                                   : HTTPClient::SecureTransportImpl::TLS)
                     : HTTPClient::SecureTransportImpl::NONE,
      timeout,
      timeout,
      /*clientCertPath=*/"",
      /*clientKeyPath=*/"");

  XLOG(INFO) << "Sending GET for " << urlStr;
  co_await HTTPClient::get(endpointSession,
                           std::move(url),
                           std::move(reader),
                           timeout,
                           getDefaultHeaders());
  endpointSession->initiateDrain();
}
