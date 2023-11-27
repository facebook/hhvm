/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "StaticHandler.h"

#include <folly/FileUtil.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/io/async/EventBaseManager.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

using namespace proxygen;

namespace StaticService {

/**
 * Handles requests by serving the file named in path.  Only supports GET.
 * reads happen in a CPU thread pool since read(2) is blocking.
 * If egress pauses, file reading is also paused.
 */

void StaticHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
  error_ = false;
  if (headers->getMethod() != HTTPMethod::GET) {
    ResponseBuilder(downstream_)
        .status(400, "Bad method")
        .body("Only GET is supported")
        .sendWithEOM();
    return;
  }
  // a real webserver would validate this path didn't contain malicious
  // characters like '//' or '..'
  try {
    // + 1 to kill leading /
    file_ = std::make_unique<folly::File>(
        headers->getPathAsStringPiece().subpiece(1));
  } catch (const std::system_error& ex) {
    ResponseBuilder(downstream_)
        .status(404, "Not Found")
        .body(folly::to<std::string>("Could not find ",
                                     headers->getPathAsStringPiece(),
                                     " ex=",
                                     folly::exceptionStr(ex)))
        .sendWithEOM();
    return;
  }
  ResponseBuilder(downstream_).status(200, "Ok").send();
  // use a CPU executor since read(2) of a file can block
  readFileScheduled_ = true;
  folly::getUnsafeMutableGlobalCPUExecutor()->add(
      std::bind(&StaticHandler::readFile,
                this,
                folly::EventBaseManager::get()->getEventBase()));
}

void StaticHandler::readFile(folly::EventBase* evb) {
  folly::IOBufQueue buf;
  while (file_ && !paused_) {
    // read 4k-ish chunks and foward each one to the client
    auto data = buf.preallocate(4000, 4000);
    auto rc = folly::readNoInt(file_->fd(), data.first, data.second);
    if (rc < 0) {
      // error
      VLOG(4) << "Read error=" << rc;
      file_.reset();
      evb->runInEventBaseThread([this] {
        LOG(ERROR) << "Error reading file";
        downstream_->sendAbort();
      });
      break;
    } else if (rc == 0) {
      // done
      file_.reset();
      VLOG(4) << "Read EOF";
      evb->runInEventBaseThread([this] {
        if (!error_) {
          ResponseBuilder(downstream_).sendWithEOM();
        }
      });
      break;
    } else {
      buf.postallocate(rc);
      evb->runInEventBaseThread([this, body = buf.move()]() mutable {
        if (!error_) {
          ResponseBuilder(downstream_).body(std::move(body)).send();
        }
      });
    }
  }

  // Notify the request thread that we terminated the readFile loop
  evb->runInEventBaseThread([this] {
    readFileScheduled_ = false;
    if (!checkForCompletion() && !paused_) {
      VLOG(4) << "Resuming deferred readFile";
      onEgressResumed();
    }
  });
}

void StaticHandler::onEgressPaused() noexcept {
  // This will terminate readFile soon
  VLOG(4) << "StaticHandler paused";
  paused_ = true;
}

void StaticHandler::onEgressResumed() noexcept {
  VLOG(4) << "StaticHandler resumed";
  paused_ = false;
  // If readFileScheduled_, it will reschedule itself
  if (!readFileScheduled_ && file_) {
    readFileScheduled_ = true;
    folly::getUnsafeMutableGlobalCPUExecutor()->add(
        std::bind(&StaticHandler::readFile,
                  this,
                  folly::EventBaseManager::get()->getEventBase()));
  } else {
    VLOG(4) << "Deferred scheduling readFile";
  }
}

void StaticHandler::onBody(std::unique_ptr<folly::IOBuf> /*body*/) noexcept {
  // ignore, only support GET
}

void StaticHandler::onEOM() noexcept {
}

void StaticHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {
  // handler doesn't support upgrades
}

void StaticHandler::requestComplete() noexcept {
  finished_ = true;
  paused_ = true;
  checkForCompletion();
}

void StaticHandler::onError(ProxygenError /*err*/) noexcept {
  error_ = true;
  finished_ = true;
  paused_ = true;
  checkForCompletion();
}

bool StaticHandler::checkForCompletion() {
  if (finished_ && !readFileScheduled_) {
    VLOG(4) << "deleting StaticHandler";
    delete this;
    return true;
  }
  return false;
}

} // namespace StaticService
