/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/SSLContext.h>
#include <folly/portability/GFlags.h>
#include <folly/ssl/Init.h>
#include <proxygen/httpclient/samples/curl/CurlClient.h>
#include <proxygen/lib/http/HTTPConnector.h>

using namespace CurlService;
using namespace folly;
using namespace proxygen;

DEFINE_string(http_method,
              "GET",
              "HTTP method to use. GET or POST are supported");
DEFINE_string(url,
              "https://github.com/facebook/proxygen",
              "URL to perform the HTTP method against");
DEFINE_string(input_filename, "", "Filename to read from for POST requests");
DEFINE_int32(http_client_connect_timeout,
             1000,
             "connect timeout in milliseconds");
DEFINE_int32(http_client_request_timeout,
             5000,
             "request timeout in milliseconds");
DEFINE_string(ca_path,
              "/etc/ssl/certs/ca-certificates.crt",
              "Path to trusted CA file"); // default for Ubuntu 14.04
DEFINE_string(cert_path, "", "Path to client certificate file");
DEFINE_string(key_path, "", "Path to client private key file");
DEFINE_string(next_protos,
              "h2,h2-14,spdy/3.1,spdy/3,http/1.1",
              "Next protocol string for NPN/ALPN");
DEFINE_string(plaintext_proto, "", "plaintext protocol");
DEFINE_int32(recv_window, 65536, "Flow control receive window for h2/spdy");
DEFINE_bool(h2c, true, "Attempt HTTP/1.1 -> HTTP/2 upgrade");
DEFINE_string(headers, "", "List of N=V headers separated by ,");
DEFINE_string(proxy, "", "HTTP proxy URL");
DEFINE_bool(log_response,
            true,
            "Whether to log the response content to stderr");

int main(int argc, char* argv[]) {
#if FOLLY_HAVE_LIBGFLAGS
  // Enable glog logging to stderr by default.
  gflags::SetCommandLineOptionWithMode(
      "logtostderr", "1", gflags::SET_FLAGS_DEFAULT);
#endif
  auto _ = folly::Init(&argc, &argv, false);
  folly::ssl::init();

  EventBase evb;
  URL url(FLAGS_url);
  URL proxy(FLAGS_proxy);

  auto httpMethod = stringToMethod(FLAGS_http_method);
  if (!httpMethod) {
    LOG(ERROR) << "Unsupported http_method=" << FLAGS_http_method;
    return EXIT_FAILURE;
  }

  if (*httpMethod == HTTPMethod::POST) {
    try {
      File f(FLAGS_input_filename);
      (void)f;
    } catch (const std::system_error& se) {
      LOG(ERROR) << "Couldn't open file for POST method";
      LOG(ERROR) << se.what();
      return EXIT_FAILURE;
    }
  }

  HTTPHeaders headers = CurlClient::parseHeaders(FLAGS_headers);

  CurlClient curlClient(&evb,
                        *httpMethod,
                        url,
                        FLAGS_proxy.empty() ? nullptr : &proxy,
                        headers,
                        FLAGS_input_filename,
                        FLAGS_h2c);
  curlClient.setFlowControlSettings(FLAGS_recv_window);
  curlClient.setLogging(FLAGS_log_response);
  curlClient.setHeadersLogging(FLAGS_log_response);

  SocketAddress addr;
  if (!FLAGS_proxy.empty()) {
    addr = SocketAddress(proxy.getHost(), proxy.getPort(), true);
  } else {
    addr = SocketAddress(url.getHost(), url.getPort(), true);
  }
  LOG(INFO) << "Trying to connect to " << addr;

  HTTPConnector connector(
      &curlClient,
      WheelTimerInstance(
          std::chrono::milliseconds(FLAGS_http_client_request_timeout), &evb));
  if (!FLAGS_plaintext_proto.empty()) {
    connector.setPlaintextProtocol(FLAGS_plaintext_proto);
  }
  static const SocketOptionMap opts{{{SOL_SOCKET, SO_REUSEADDR}, 1}};

  if (url.isSecure()) {
    curlClient.initializeSsl(
        FLAGS_ca_path, FLAGS_next_protos, FLAGS_cert_path, FLAGS_key_path);
    connector.connectSSL(
        &evb,
        addr,
        curlClient.getSSLContext(),
        nullptr,
        std::chrono::milliseconds(FLAGS_http_client_connect_timeout),
        opts,
        folly::AsyncSocket::anyAddress(),
        curlClient.getServerName());
  } else {
    connector.connect(
        &evb,
        addr,
        std::chrono::milliseconds(FLAGS_http_client_connect_timeout),
        opts);
  }

  evb.loop();

  return EXIT_SUCCESS;
}
