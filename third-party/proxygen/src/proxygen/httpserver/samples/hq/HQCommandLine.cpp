/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/samples/hq/HQCommandLine.h>

#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/net/NetworkSocket.h>
#include <folly/portability/GFlags.h>
#include <proxygen/httpclient/samples/curl/CurlClient.h>
#include <proxygen/lib/http/SynchronizedLruQuicPskCache.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/transport/PersistentQuicPskCache.h>
#include <quic/QuicConstants.h>

DEFINE_string(host, "::1", "HQ server hostname/IP");
DEFINE_int32(port, 6666, "HQ server port");
DEFINE_int32(threads, 0, "QUIC Server threads, 0 = nCPUs");
DEFINE_int32(h2port, 6667, "HTTP/2 server port");
DEFINE_string(
    local_address,
    "",
    "Local Address to bind to. Client only. Format should be ip:port");
DEFINE_string(mode, "server", "Mode to run in: 'client' or 'server'");
DEFINE_string(body, "", "Filename to read from for POST requests");
DEFINE_string(path,
              "/",
              "(HQClient) url-path to send the request to, "
              "or a comma separated list of paths to fetch in parallel");
DEFINE_string(connect_to_address,
              "",
              "(HQClient) Override IP address to connect to instead of "
              "resolving the host field");
DEFINE_int32(connect_timeout, 2000, "(HQClient) connect timeout in ms");
DEFINE_string(httpversion, "1.1", "HTTP version string");
DEFINE_string(protocol, "", "HQ protocol version e.g. h3-29 or hq-fb-05");
DEFINE_int64(quic_version, 0, "QUIC version to use. 0 is default");
DEFINE_bool(use_version, true, "Use set QUIC version as first version");
DEFINE_string(logdir, "/tmp/logs", "Directory to store connection logs");
DEFINE_string(outdir, "", "Directory to store responses");
DEFINE_bool(log_response,
            true,
            "Whether to log the response content to stderr");
DEFINE_bool(log_response_headers,
            false,
            "Whether to log the response headers to stderr");
DEFINE_bool(
    log_run_time,
    false,
    "Whether to log the duration for which the client/server was running");
DEFINE_bool(sequential,
            false,
            "Whether to make requests sequentially or in parallel when "
            "multiple paths are provided");
DEFINE_uint32(gap_ms, 0, "Time between consecutive requests in milliseconds");
DEFINE_string(congestion, "cubic", "newreno/cubic/bbr/none");
DEFINE_int32(conn_flow_control, 1024 * 1024 * 10, "Connection flow control");
DEFINE_int32(stream_flow_control, 256 * 1024, "Stream flow control");
DEFINE_int32(max_receive_packet_size,
             quic::kDefaultUDPReadBufferSize,
             "Max UDP packet size Quic can receive");
DEFINE_int64(rate_limit, -1, "Connection rate limit per second per thread");

DEFINE_uint32(num_gro_buffers,
              quic::kDefaultNumGROBuffers,
              "Number of GRO buffers");

DEFINE_int32(txn_timeout, 120000, "HTTP Transaction Timeout");
DEFINE_string(headers, "", "List of N=V headers separated by ,");
DEFINE_bool(pacing, false, "Whether to enable pacing on HQServer");
DEFINE_int32(pacing_timer_tick_interval_us, 200, "Pacing timer resolution");
DEFINE_string(psk_file, "", "Cache file to use for QUIC psks");
DEFINE_bool(early_data, false, "Whether to use 0-rtt");
DEFINE_uint32(quic_batching_mode,
              static_cast<uint32_t>(quic::QuicBatchingMode::BATCHING_MODE_NONE),
              "QUIC batching mode");
DEFINE_bool(quic_use_thread_local_batching, false, "Use thread local batching");
DEFINE_uint32(quic_thread_local_delay_us,
              1000,
              "Thread local delay in microseconds");
DEFINE_uint32(quic_batch_size,
              quic::kDefaultQuicMaxBatchSize,
              "Maximum number of packets that can be batched in Quic");
DEFINE_string(cert, "", "Certificate file path");
DEFINE_string(key, "", "Private key file path");
DEFINE_string(client_auth_mode, "none", "Client authentication mode");
DEFINE_string(qlogger_path,
              "",
              "Path to the directory where qlog files"
              "will be written. File is called <CID>.qlog");
DEFINE_bool(pretty_json, true, "Whether to use pretty json for QLogger output");
DEFINE_bool(connect_udp, false, "Whether or not to use connected udp sockets");
DEFINE_uint32(max_cwnd_mss,
              quic::kLargeMaxCwndInMss,
              "Max cwnd in unit of mss");
DEFINE_bool(migrate_client,
            false,
            "(HQClient) Should the HQClient make two sets of requests and "
            "switch sockets in the middle.");
DEFINE_bool(use_inplace_write,
            false,
            "Transport use inplace packet build and socket writing");

DEFINE_bool(send_knob_frame,
            false,
            "Send a Knob Frame to the peer when a QUIC connection is "
            "established successfully");

DEFINE_string(transport_knobs,
              "",
              "If send_knob_frame is set, this is the default transport knobs"
              " sent to peer");
DEFINE_bool(use_ack_receive_timestamps,
            false,
            "Replace the ACK frame with ACK_RECEIVE_TIMESTAMPS frame"
            "which carries the received packet timestamps");
DEFINE_uint32(
    max_ack_receive_timestamps_to_send,
    quic::kMaxReceivedPktsTimestampsStored,
    "Controls how many packet receieve timestamps the peer should send");

namespace quic::samples {

std::ostream& operator<<(std::ostream& o, const HQMode& m) {
  o << "mode=";
  switch (m) {
    case HQMode::CLIENT:
      o << "client";
      break;
    case HQMode::SERVER:
      o << "server";
      break;
    default:
      o << "unknown (val=" << static_cast<uint32_t>(m) << ")";
  }
  return o;
}

namespace {

/*
 * Initiazliation and validation functions.
 *
 * The pattern is to collect flags into the HQToolParamsBuilderFromCmdline
 * object and then to validate it. Rationale of validating the options AFTER all
 * the options have been collected: some combinations of transport, http and
 * partial reliability options are invalid. It is simpler to collect the options
 * first and to validate the combinations later.
 *
 */
void initializeCommonSettings(HQToolParams& hqParams) {
  // General section
  if (FLAGS_mode == "server") {
    CHECK(FLAGS_local_address.empty())
        << "local_address only allowed in client mode";
    hqParams.setMode(HQMode::SERVER);
    hqParams.logprefix = "server";
    auto& serverParams = boost::get<HQToolServerParams>(hqParams.params);
    serverParams.host = FLAGS_host;
    serverParams.port = FLAGS_port;
    serverParams.serverThreads = FLAGS_threads;
    serverParams.localAddress =
        folly::SocketAddress(serverParams.host, serverParams.port, true);
  } else if (FLAGS_mode == "client") {
    hqParams.setMode(HQMode::CLIENT);
    hqParams.logprefix = "client";
    auto& clientParams = boost::get<HQToolClientParams>(hqParams.params);
    clientParams.host = FLAGS_host;
    clientParams.port = FLAGS_port;
    if (FLAGS_connect_to_address.empty()) {
      clientParams.remoteAddress =
          folly::SocketAddress(clientParams.host, clientParams.port, true);
    } else {
      clientParams.remoteAddress = folly::SocketAddress(
          FLAGS_connect_to_address, clientParams.port, false);
    }
    if (!FLAGS_local_address.empty()) {
      clientParams.localAddress = folly::SocketAddress();
      clientParams.localAddress->setFromLocalIpPort(FLAGS_local_address);
    }
    clientParams.outdir = FLAGS_outdir;
  }
}

void initializeTransportSettings(HQToolParams& hqUberParams) {
  HQBaseParams& hqParams = hqUberParams.baseParams();
  if (FLAGS_quic_version != 0) {
    auto quicVersion = static_cast<quic::QuicVersion>(FLAGS_quic_version);
    bool useVersionFirst = FLAGS_use_version;
    if (useVersionFirst) {
      hqParams.quicVersions.insert(hqParams.quicVersions.begin(), quicVersion);
    } else {
      hqParams.quicVersions.push_back(quicVersion);
    }
  }

  if (!FLAGS_protocol.empty()) {
    hqParams.protocol = FLAGS_protocol;
    hqParams.supportedAlpns = {hqParams.protocol};
  }

  hqParams.transportSettings.advertisedInitialConnectionFlowControlWindow =
      FLAGS_conn_flow_control;
  hqParams.transportSettings.advertisedInitialBidiLocalStreamFlowControlWindow =
      FLAGS_stream_flow_control;
  hqParams.transportSettings
      .advertisedInitialBidiRemoteStreamFlowControlWindow =
      FLAGS_stream_flow_control;
  hqParams.transportSettings.advertisedInitialUniStreamFlowControlWindow =
      FLAGS_stream_flow_control;
  hqParams.congestionControlName = FLAGS_congestion;
  hqParams.congestionControl =
      quic::congestionControlStrToType(FLAGS_congestion);
  if (hqParams.congestionControl) {
    hqParams.transportSettings.defaultCongestionController =
        hqParams.congestionControl.value();
  }
  hqParams.transportSettings.maxRecvPacketSize = FLAGS_max_receive_packet_size;
  hqParams.transportSettings.numGROBuffers_ = FLAGS_num_gro_buffers;
  hqParams.transportSettings.pacingEnabled = FLAGS_pacing;
  if (hqParams.transportSettings.pacingEnabled) {
    hqParams.transportSettings.pacingTickInterval =
        std::chrono::microseconds(FLAGS_pacing_timer_tick_interval_us);
  }
  hqParams.transportSettings.batchingMode =
      quic::getQuicBatchingMode(FLAGS_quic_batching_mode);
  hqParams.transportSettings.useThreadLocalBatching =
      FLAGS_quic_use_thread_local_batching;
  hqParams.transportSettings.threadLocalDelay =
      std::chrono::microseconds(FLAGS_quic_thread_local_delay_us);
  hqParams.transportSettings.maxBatchSize = FLAGS_quic_batch_size;
  if (hqUberParams.mode == HQMode::CLIENT) {
    // There is no good reason to keep the socket around for a drain period for
    // a commandline client
    hqParams.transportSettings.shouldDrain = false;
    hqParams.transportSettings.attemptEarlyData = FLAGS_early_data;
  }
  hqParams.transportSettings.connectUDP = FLAGS_connect_udp;
  hqParams.transportSettings.maxCwndInMss = FLAGS_max_cwnd_mss;
  if (hqUberParams.mode == HQMode::SERVER && FLAGS_use_inplace_write) {
    hqParams.transportSettings.dataPathType =
        quic::DataPathType::ContinuousMemory;
  }
  if (FLAGS_rate_limit >= 0) {
    CHECK(hqUberParams.mode == HQMode::SERVER);
    boost::get<HQToolServerParams>(hqUberParams.params).rateLimitPerThread =
        FLAGS_rate_limit;

    std::array<uint8_t, kRetryTokenSecretLength> secret;
    folly::Random::secureRandom(secret.data(), secret.size());
    hqParams.transportSettings.retryTokenSecret = secret;
  }
  if (hqUberParams.mode == HQMode::CLIENT) {
    boost::get<HQToolClientParams>(hqUberParams.params).connectTimeout =
        std::chrono::milliseconds(FLAGS_connect_timeout);
  }
  hqParams.sendKnobFrame = FLAGS_send_knob_frame;
  if (hqParams.sendKnobFrame) {
    hqParams.transportSettings.knobs.push_back({kDefaultQuicTransportKnobSpace,
                                                kDefaultQuicTransportKnobId,
                                                FLAGS_transport_knobs});
  }
  hqParams.transportSettings.shouldRecvBatch = true;
  hqParams.transportSettings.maxRecvBatchSize = 32;
  hqParams.transportSettings.shouldUseRecvmmsgForBatchRecv = true;
  hqParams.transportSettings.advertisedInitialMaxStreamsBidi = 100;
  hqParams.transportSettings.advertisedInitialMaxStreamsUni = 100;

  if (FLAGS_use_ack_receive_timestamps) {
    hqParams.transportSettings.maybeAckReceiveTimestampsConfigSentToPeer.assign(
        {.maxReceiveTimestampsPerAck = FLAGS_max_ack_receive_timestamps_to_send,
         .receiveTimestampsExponent = kDefaultReceiveTimestampsExponent});
  }
} // initializeTransportSettings

void initializeHttpServerSettings(HQToolServerParams& hqParams) {
  // HTTP section
  // NOTE: handler factories are assigned by H2Server class
  // before starting.
  hqParams.h2port = FLAGS_h2port;
  hqParams.localH2Address =
      folly::SocketAddress(hqParams.host, hqParams.h2port, true);
  hqParams.httpServerThreads = FLAGS_threads;
  hqParams.httpServerIdleTimeout = std::chrono::milliseconds(60000);
  hqParams.httpServerShutdownOn = {SIGINT, SIGTERM};
  hqParams.httpServerEnableContentCompression = false;
  hqParams.h2cEnabled = false;
  hqParams.httpVersion.parse(FLAGS_httpversion);
  hqParams.txnTimeout = std::chrono::milliseconds(FLAGS_txn_timeout);
} // initializeHttpServerSettings

void initializeHttpClientSettings(HQToolClientParams& hqParams) {
  folly::split(',', FLAGS_path, hqParams.httpPaths);
  hqParams.httpBody = FLAGS_body;
  hqParams.httpMethod = hqParams.httpBody.empty() ? proxygen::HTTPMethod::GET
                                                  : proxygen::HTTPMethod::POST;

  // parse HTTP headers
  auto httpHeadersString = FLAGS_headers;
  hqParams.httpHeaders =
      CurlService::CurlClient::parseHeaders(httpHeadersString);

  // Set the host header
  if (!hqParams.httpHeaders.exists(proxygen::HTTP_HEADER_HOST)) {
    hqParams.httpHeaders.set(proxygen::HTTP_HEADER_HOST, hqParams.host);
  }

  hqParams.logResponse = FLAGS_log_response;
  hqParams.logResponseHeaders = FLAGS_log_response_headers;
  hqParams.sendRequestsSequentially = FLAGS_sequential;
  hqParams.gapBetweenRequests = std::chrono::milliseconds(FLAGS_gap_ms);

  hqParams.earlyData = FLAGS_early_data;
  hqParams.migrateClient = FLAGS_migrate_client;
  hqParams.txnTimeout = std::chrono::milliseconds(FLAGS_txn_timeout);
  hqParams.httpVersion.parse(FLAGS_httpversion);
} // initializeHttpClientSettings

void initializeQLogSettings(HQBaseParams& hqParams) {
  hqParams.qLoggerPath = FLAGS_qlogger_path;
  hqParams.prettyJson = FLAGS_pretty_json;
} // initializeQLogSettings

void initializeFizzSettings(HQBaseParams& hqParams) {
  hqParams.certificateFilePath = FLAGS_cert;
  hqParams.keyFilePath = FLAGS_key;
  hqParams.pskFilePath = FLAGS_psk_file;
  if (!FLAGS_psk_file.empty()) {
    hqParams.pskCache = std::make_shared<proxygen::PersistentQuicPskCache>(
        FLAGS_psk_file,
        wangle::PersistentCacheConfig::Builder()
            .setCapacity(1000)
            .setSyncInterval(std::chrono::seconds(1))
            .build());
  } else {
    hqParams.pskCache =
        std::make_shared<proxygen::SynchronizedLruQuicPskCache>(1000);
  }

  if (FLAGS_client_auth_mode == "none") {
    hqParams.clientAuth = fizz::server::ClientAuthMode::None;
  } else if (FLAGS_client_auth_mode == "optional") {
    hqParams.clientAuth = fizz::server::ClientAuthMode::Optional;
  } else if (FLAGS_client_auth_mode == "required") {
    hqParams.clientAuth = fizz::server::ClientAuthMode::Required;
  }

} // initializeFizzSettings

HQInvalidParams validate(const HQToolParams& params) {
  HQInvalidParams invalidParams;
#define INVALID_PARAM(param, error)                                           \
  do {                                                                        \
    HQInvalidParam invalid = {.name = #param,                                 \
                              .value = folly::to<std::string>(FLAGS_##param), \
                              .errorMsg = (error)};                           \
    invalidParams.push_back(invalid);                                         \
  } while (false);

  // Validate the common settings
  if (!(params.mode == HQMode::CLIENT || params.mode == HQMode::SERVER)) {
    INVALID_PARAM(mode, "only client/server are supported");
  }

  // In the client mode, host/port are required
  if (params.mode == HQMode::CLIENT) {
    auto& clientParams = boost::get<HQToolClientParams>(params.params);
    if (clientParams.host.empty()) {
      INVALID_PARAM(host, "HQClient expected --host");
    }
    if (clientParams.port == 0) {
      INVALID_PARAM(port, "HQClient expected --port");
    }
  }

  // Validate the transport section
  if (folly::to<uint16_t>(FLAGS_max_receive_packet_size) <
      quic::kDefaultUDPSendPacketLen) {
    INVALID_PARAM(
        max_receive_packet_size,
        folly::to<std::string>("max_receive_packet_size needs to be at least ",
                               quic::kDefaultUDPSendPacketLen));
  }

  auto& baseParams = params.baseParams();
  if (!baseParams.congestionControlName.empty()) {
    if (!baseParams.congestionControl) {
      INVALID_PARAM(congestion, "unrecognized congestion control");
    }
  }
  // Validate the HTTP section
  if (params.mode == HQMode::SERVER) {
  }

  return invalidParams;
#undef INVALID_PARAM
}
} // namespace

HQToolParamsBuilderFromCmdline::HQToolParamsBuilderFromCmdline(
    initializer_list initial) {
  // Save the values of the flags, so that changing
  // flags values is safe
  gflags::FlagSaver saver;

  for (auto& kv : initial) {
    gflags::SetCommandLineOptionWithMode(
        kv.first.c_str(),
        kv.second.c_str(),
        gflags::FlagSettingMode::SET_FLAGS_VALUE);
  }

  hqParams_.logdir = FLAGS_logdir;
  hqParams_.logRuntime = FLAGS_log_run_time;

  initializeCommonSettings(hqParams_);

  initializeTransportSettings(hqParams_);

  if (hqParams_.mode == HQMode::CLIENT) {
    initializeHttpClientSettings(
        boost::get<HQToolClientParams>(hqParams_.params));
  } else {
    initializeHttpServerSettings(
        boost::get<HQToolServerParams>(hqParams_.params));
  }

  initializeQLogSettings(hqParams_.baseParams());

  initializeFizzSettings(hqParams_.baseParams());

  for (auto& err : validate(hqParams_)) {
    invalidParams_.push_back(err);
  }
}

bool HQToolParamsBuilderFromCmdline::valid() const noexcept {
  return invalidParams_.empty();
}

const HQInvalidParams& HQToolParamsBuilderFromCmdline::invalidParams()
    const noexcept {
  return invalidParams_;
}

HQToolParams HQToolParamsBuilderFromCmdline::build() noexcept {
  return hqParams_;
}

const folly::Expected<HQToolParams, HQInvalidParams>
initializeParamsFromCmdline(
    HQToolParamsBuilderFromCmdline::initializer_list defaultValues) {
  auto builder =
      std::make_shared<HQToolParamsBuilderFromCmdline>(defaultValues);

  // Wrap up and return
  if (builder->valid()) {
    return builder->build();
  } else {
    auto errors = builder->invalidParams();
    return folly::makeUnexpected(errors);
  }
}

} // namespace quic::samples
