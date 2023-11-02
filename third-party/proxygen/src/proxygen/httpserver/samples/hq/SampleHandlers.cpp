/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/samples/hq/SampleHandlers.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <proxygen/lib/utils/Logging.h>
#include <string>

namespace {
std::atomic<bool> shouldPassHealthChecks{true};
}

DEFINE_string(static_root,
              "",
              "Path to serve static files from. Disabled if empty.");

namespace quic::samples {

using namespace proxygen;

HTTPTransactionHandler* Dispatcher::getRequestHandler(HTTPMessage* msg) {
  DCHECK(msg);
  auto path = msg->getPathAsStringPiece();
  if (path == "/" || path == "/echo") {
    return new EchoHandler(params_);
  }
  if (path == "/continue") {
    return new ContinueHandler(params_);
  }
  if (path.size() > 1 && path[0] == '/' && std::isdigit(path[1])) {
    return new RandBytesGenHandler(params_);
  }
  if (path == "/status") {
    return new HealthCheckHandler(shouldPassHealthChecks, params_);
  }
  if (path == "/status_ok") {
    shouldPassHealthChecks = true;
    return new HealthCheckHandler(true, params_);
  }
  if (path == "/status_fail") {
    shouldPassHealthChecks = false;
    return new HealthCheckHandler(true, params_);
  }
  if (path == "/wait" || path == "/release") {
    return new WaitReleaseHandler(
        folly::EventBaseManager::get()->getEventBase(), params_);
  }
  if (boost::algorithm::starts_with(path, "/chunked")) {
    return new ChunkedHandler(params_,
                              folly::EventBaseManager::get()->getEventBase());
  }
  if (boost::algorithm::starts_with(path, "/push")) {
    return new ServerPushHandler(params_);
  }

  if (boost::algorithm::starts_with(path, "/webtransport/devious-baton")) {
    return new DeviousBatonHandler(
        params_, folly::EventBaseManager::get()->getEventBase());
  }

  if (!FLAGS_static_root.empty()) {
    return new StaticFileHandler(params_, FLAGS_static_root);
  }
  if (boost::algorithm::starts_with(path, "/delay")) {
    return new DelayHandler(params_,
                            folly::EventBaseManager::get()->getEventBase());
  }
  return new DummyHandler(params_);
}

class WaitReleaseHandler;

std::unordered_map<uint, WaitReleaseHandler*>&
WaitReleaseHandler::getWaitingHandlers() {
  static std::unordered_map<uint, WaitReleaseHandler*> waitingHandlers;
  return waitingHandlers;
}

std::mutex& WaitReleaseHandler::getMutex() {
  static std::mutex mutex;
  return mutex;
}

void WaitReleaseHandler::onHeadersComplete(
    std::unique_ptr<proxygen::HTTPMessage> msg) noexcept {
  VLOG(10) << "WaitReleaseHandler::onHeadersComplete";
  msg->dumpMessage(2);
  path_ = msg->getPath();
  auto idstr = msg->getQueryParam("id");

  if (msg->getMethod() != proxygen::HTTPMethod::GET ||
      idstr == proxygen::empty_string ||
      (path_ != "/wait" && path_ != "/release")) {
    sendErrorResponse("bad request\n");
    return;
  }

  auto id = folly::tryTo<uint>(idstr);
  if (!id.hasValue() || id.value() == 0) {
    sendErrorResponse("invalid id\n");
    return;
  }

  id_ = id.value();

  txn_->setIdleTimeout(std::chrono::milliseconds(120000));
  std::lock_guard<std::mutex> g(getMutex());
  auto& waitingHandlers = getWaitingHandlers();
  if (path_ == "/wait") {
    auto waitHandler = waitingHandlers.find(id_);
    if (waitHandler != waitingHandlers.end()) {
      sendErrorResponse("id already exists\n");
      return;
    }
    waitingHandlers.insert(std::make_pair(id_, this));
    sendOkResponse("waiting\n", false /* eom */);
  } else if (path_ == "/release") {
    auto waitHandler = waitingHandlers.find(id.value());
    if (waitHandler == waitingHandlers.end()) {
      sendErrorResponse("id does not exist\n");
      return;
    }
    waitHandler->second->release();
    waitingHandlers.erase(waitHandler);
    sendOkResponse("released\n", true /* eom */);
  }
}

void WaitReleaseHandler::maybeCleanup() {
  if (path_ == "/wait" && id_ != 0) {
    std::lock_guard<std::mutex> g(getMutex());
    auto& waitingHandlers = getWaitingHandlers();
    auto waitHandler = waitingHandlers.find(id_);
    if (waitHandler != waitingHandlers.end()) {
      waitingHandlers.erase(waitHandler);
    }
  }
}
// ServerPushHandler methods
//

class ServerPushHandler;

void ServerPushHandler::onHeadersComplete(
    std::unique_ptr<proxygen::HTTPMessage> msg) noexcept {
  VLOG(10) << "ServerPushHandler::" << __func__;
  msg->dumpMessage(2);
  path_ = msg->getPath();

  if (msg->getMethod() != proxygen::HTTPMethod::GET) {
    LOG(ERROR) << "Method not supported";
    sendErrorResponse("bad request\n");
    return;
  }

  VLOG(2) << "Received GET request for " << path_ << " at: "
          << std::chrono::duration_cast<std::chrono::microseconds>(
                 std::chrono::steady_clock::now().time_since_epoch())
                 .count();

  std::string gPushResponseBody;
  std::vector<std::string> pathPieces;
  std::string path = path_;
  boost::split(pathPieces, path, boost::is_any_of("/"));
  int responseSize = 0;
  int numResponses = 1;

  if (pathPieces.size() > 2) {
    auto sizeFromPath = folly::tryTo<int>(pathPieces[2]);
    responseSize = sizeFromPath.value_or(0);
    if (responseSize != 0) {
      VLOG(2) << "Requested a response size of " << responseSize;
      gPushResponseBody = std::string(responseSize, 'a');
    }
  }

  if (pathPieces.size() > 3) {
    auto numResponsesFromPath = folly::tryTo<int>(pathPieces[3]);
    numResponses = numResponsesFromPath.value_or(1);
    VLOG(2) << "Requested a repeat count of " << numResponses;
  }

  for (int i = 0; i < numResponses; ++i) {
    VLOG(2) << "Sending push txn " << i << "/" << numResponses;

    // Create a URL for the pushed resource
    auto pushedResourceUrl =
        folly::to<std::string>(msg->getURL(), "/", "pushed", i);

    // Create a pushed transaction and handler
    auto pushedTxn = txn_->newPushedTransaction(&pushTxnHandler_);

    if (!pushedTxn) {
      LOG(ERROR) << "Could not create push txn; stop pushing";
      break;
    }

    // Send a promise for the pushed resource
    sendPushPromise(pushedTxn, pushedResourceUrl);

    // Send the push response
    sendPushResponse(
        pushedTxn, pushedResourceUrl, gPushResponseBody, true /* eom */);
  }

  // Send the response to the original get request
  sendOkResponse("I AM THE REQUEST RESPONSE AND I AM RESPONSIBLE\n",
                 true /* eom */);
}

void ServerPushHandler::sendPushPromise(proxygen::HTTPTransaction* txn,
                                        const std::string& pushedResourceUrl) {
  VLOG(10) << "ServerPushHandler::" << __func__;
  proxygen::HTTPMessage promise;
  promise.setMethod("GET");
  promise.setURL(pushedResourceUrl);
  promise.setVersionString(getHttpVersion());
  promise.setIsChunked(true);
  txn->sendHeaders(promise);

  VLOG(2) << "Sent push promise for " << pushedResourceUrl << " at: "
          << std::chrono::duration_cast<std::chrono::microseconds>(
                 std::chrono::steady_clock::now().time_since_epoch())
                 .count();
}

void ServerPushHandler::sendPushResponse(proxygen::HTTPTransaction* pushTxn,
                                         const std::string& pushedResourceUrl,
                                         const std::string& pushedResourceBody,
                                         bool eom) {
  VLOG(10) << "ServerPushHandler::" << __func__;
  proxygen::HTTPMessage resp = createHttpResponse(200, "OK");
  resp.setWantsKeepalive(true);
  resp.setIsChunked(true);
  pushTxn->sendHeaders(resp);

  std::string responseStr =
      "I AM THE PUSHED RESPONSE AND I AM NOT RESPONSIBLE: " +
      pushedResourceBody;
  pushTxn->sendBody(folly::IOBuf::copyBuffer(responseStr));

  VLOG(2) << "Sent push response for " << pushedResourceUrl << " at: "
          << std::chrono::duration_cast<std::chrono::microseconds>(
                 std::chrono::steady_clock::now().time_since_epoch())
                 .count();

  if (eom) {
    pushTxn->sendEOM();
    VLOG(2) << "Sent EOM for " << pushedResourceUrl << " at: "
            << std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::steady_clock::now().time_since_epoch())
                   .count();
  }
}

void ServerPushHandler::sendErrorResponse(const std::string& body) {
  proxygen::HTTPMessage resp = createHttpResponse(400, "ERROR");
  resp.setWantsKeepalive(false);
  txn_->sendHeaders(resp);
  txn_->sendBody(folly::IOBuf::copyBuffer(body));
  txn_->sendEOM();
}

void ServerPushHandler::sendOkResponse(const std::string& body, bool eom) {
  VLOG(10) << "ServerPushHandler::" << __func__ << ": sending " << body.length()
           << " bytes";
  proxygen::HTTPMessage resp = createHttpResponse(200, "OK");
  resp.setWantsKeepalive(true);
  resp.setIsChunked(true);
  txn_->sendHeaders(resp);
  txn_->sendBody(folly::IOBuf::copyBuffer(body));
  if (eom) {
    txn_->sendEOM();
  }
}

void ServerPushHandler::onBody(
    std::unique_ptr<folly::IOBuf> /*chain*/) noexcept {
  VLOG(10) << "ServerPushHandler::" << __func__ << " - ignoring";
}

void ServerPushHandler::onEOM() noexcept {
  VLOG(10) << "ServerPushHandler::" << __func__ << " - ignoring";
}

void ServerPushHandler::onError(const proxygen::HTTPException& error) noexcept {
  VLOG(10) << "ServerPushHandler::onError error=" << error.what();
}

void DeviousBatonHandler::onHeadersComplete(
    std::unique_ptr<proxygen::HTTPMessage> msg) noexcept {
  VLOG(10) << "WebtransHandler::" << __func__;
  msg->dumpMessage(2);

  if (msg->getMethod() != proxygen::HTTPMethod::CONNECT) {
    LOG(ERROR) << "Method not supported";
    proxygen::HTTPMessage resp;
    resp.setVersionString(getHttpVersion());
    resp.setStatusCode(400);
    resp.setStatusMessage("ERROR");
    resp.setWantsKeepalive(false);
    txn_->sendHeaders(resp);
    txn_->sendEOM();
    txn_ = nullptr;
    return;
  }

  VLOG(2) << "Received CONNECT request for " << msg->getPathAsStringPiece()
          << " at: "
          << std::chrono::duration_cast<std::chrono::microseconds>(
                 std::chrono::steady_clock::now().time_since_epoch())
                 .count();

  auto status = 500;
  auto wt = txn_->getWebTransport();
  if (wt) {
    devious_.emplace(wt,
                     devious::DeviousBaton::Mode::SERVER,
                     [this](WebTransport::StreamReadHandle* readHandle) {
                       readHandle->awaitNextRead(
                           evb_, [this](auto readHandle, auto streamData) {
                             readHandler(readHandle, std::move(streamData));
                           });
                     });
    auto respCode = devious_->onRequest(*msg);
    if (!respCode) {
      status = respCode.error();
    } else {
      status = 200;
    }
  }

  // Send the response to the original get request
  proxygen::HTTPMessage resp;
  resp.setVersionString(getHttpVersion());
  resp.setStatusCode(status);
  resp.setIsChunked(true);
  if (status / 100 == 2) {
    resp.getHeaders().add("sec-webtransport-http3-draft", "draft02");
    resp.setWantsKeepalive(true);
  } else {
    resp.setWantsKeepalive(false);
    devious_.reset();
  }
  resp.dumpMessage(4);
  txn_->sendHeaders(resp);

  if (devious_) {
    devious_->start();
  } else {
    txn_->sendEOM();
    txn_ = nullptr;
  }
}

void DeviousBatonHandler::readHandler(
    WebTransport::StreamReadHandle* readHandle,
    folly::Try<WebTransport::StreamData> streamData) {
  if (streamData.hasException()) {
    VLOG(4) << "read error=" << streamData.exception().what();
  } else {
    VLOG(4) << "read data id=" << readHandle->getID();
    devious_->onStreamData(readHandle->getID(),
                           streams_[readHandle->getID()],
                           std::move(streamData->data),
                           streamData->fin);
    if (!streamData->fin) {
      readHandle->awaitNextRead(evb_, [this](auto readHandle, auto streamData) {
        readHandler(readHandle, std::move(streamData));
      });
    }
  }
}

void DeviousBatonHandler::onWebTransportBidiStream(
    HTTPCodec::StreamID id, WebTransport::BidiStreamHandle stream) noexcept {
  VLOG(4) << "New Bidi Stream=" << id;
  stream.readHandle->awaitNextRead(
      evb_, [this](auto readHandle, auto streamData) {
        readHandler(readHandle, std::move(streamData));
      });
}

void DeviousBatonHandler::onWebTransportUniStream(
    HTTPCodec::StreamID id,
    WebTransport::StreamReadHandle* readHandle) noexcept {
  VLOG(4) << "New Uni Stream=" << id;
  readHandle->awaitNextRead(evb_, [this](auto readHandle, auto streamData) {
    readHandler(readHandle, std::move(streamData));
  });
}

void DeviousBatonHandler::onWebTransportSessionClose(
    folly::Optional<uint32_t> error) noexcept {
  VLOG(4) << "Session Close error="
          << (error ? folly::to<std::string>(*error) : std::string("none"));
}

void DeviousBatonHandler::onDatagram(
    std::unique_ptr<folly::IOBuf> datagram) noexcept {
  VLOG(4) << "DeviousBatonHandler::" << __func__;
}

void DeviousBatonHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  VLOG(4) << "DeviousBatonHandler::" << __func__;
  VLOG(3) << IOBufPrinter::printHexFolly(body.get(), true);
  folly::io::Cursor cursor(body.get());

  // parse capsules
  auto leftToParse = body->computeChainDataLength();
  while (leftToParse > 0) {
    auto typeRes = quic::decodeQuicInteger(cursor, leftToParse);
    if (!typeRes) {
      VLOG(2) << "Failed to decode capsule type l=" << leftToParse;
      return;
    }
    leftToParse -= typeRes->first;
    auto capLength = quic::decodeQuicInteger(cursor, leftToParse);
    if (!capLength) {
      VLOG(2) << "Failed to decode capsule length";
      return;
    }
    leftToParse -= capLength->first;
    if (capLength->second > leftToParse) {
      VLOG(2) << "Derp";
      return;
    }
    VLOG(2) << "Parsed Capsule t=" << typeRes->second
            << " l=" << capLength->second;
    cursor.skipAtMost(capLength->second);
    leftToParse -= capLength->second;
  }
}

void DeviousBatonHandler::onEOM() noexcept {
  VLOG(4) << "DeviousBatonHandler::" << __func__;
  if (txn_ && !txn_->isEgressEOMSeen()) {
    txn_->sendEOM();
  }
}

void DeviousBatonHandler::onError(
    const proxygen::HTTPException& error) noexcept {
  VLOG(4) << "DeviousBatonHandler::onError error=" << error.what();
}
} // namespace quic::samples
