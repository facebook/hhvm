/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <wangle/ssl/TLSCredProcessor.h>

#include <folly/FileUtil.h>
#include <folly/Memory.h>
#include <folly/json/dynamic.h>
#include <folly/json/json.h>
#include <wangle/ssl/SSLUtil.h>

using namespace folly;

namespace {

constexpr std::chrono::milliseconds kCredentialPollInterval =
    std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::seconds(10));

void insertSeeds(
    const folly::dynamic& keyConfig,
    std::vector<std::string>& seedList) {
  if (!keyConfig.isArray()) {
    return;
  }
  for (const auto& seed : keyConfig) {
    seedList.push_back(seed.asString());
  }
}
} // namespace

namespace wangle {

TLSCredProcessor::TLSCredProcessor()
    : poller_(std::make_unique<FilePoller>(kCredentialPollInterval)) {}

TLSCredProcessor::TLSCredProcessor(std::chrono::milliseconds pollInterval)
    : poller_(std::make_unique<FilePoller>(pollInterval)) {}

void TLSCredProcessor::stop() {
  poller_->stop();
}

TLSCredProcessor::~TLSCredProcessor() {
  stop();
}

void TLSCredProcessor::setPollInterval(std::chrono::milliseconds pollInterval) {
  poller_->stop();
  poller_ = std::make_unique<FilePoller>(pollInterval);
  setTicketPathToWatch(ticketFile_, password_);
  setCertPathsToWatch(certFiles_);
}

void TLSCredProcessor::addTicketCallback(
    std::function<void(TLSTicketKeySeeds)> callback) {
  ticketCallbacks_.wlock()->push_back(
      folly::copy_to_shared_ptr(std::move(callback)));
}

void TLSCredProcessor::addCertCallback(std::function<void()> callback) {
  certCallbacks_.wlock()->push_back(
      folly::copy_to_shared_ptr(std::move(callback)));
}

void TLSCredProcessor::setTicketPathToWatch(
    const std::string& ticketFile,
    const folly::Optional<std::string>& password) {
  if (!ticketFile_.empty()) {
    poller_->removeFileToTrack(ticketFile_);
  }
  ticketFile_ = ticketFile;
  password_ = password;
  if (!ticketFile_.empty()) {
    auto ticketsChangedCob = [=]() {
      ticketFileUpdated(ticketFile_, password_);
    };
    poller_->addFileToTrack(ticketFile_, ticketsChangedCob);
  }
}

void TLSCredProcessor::setCertPathsToWatch(std::set<std::string> certFiles) {
  for (const auto& path : certFiles_) {
    poller_->removeFileToTrack(path);
  }
  certFiles_ = std::move(certFiles);
  if (!certFiles_.empty()) {
    auto certChangedCob = [this]() { certFileUpdated(); };
    for (const auto& path : certFiles_) {
      poller_->addFileToTrack(path, certChangedCob);
    }
  }
}

void TLSCredProcessor::ticketFileUpdated(
    const std::string& ticketFile,
    const folly::Optional<std::string>& password) noexcept {
  auto seeds = processTLSTickets(ticketFile, password);
  if (seeds) {
    for (auto& callback : ticketCallbacks_.copy()) {
      (*callback)(*seeds);
    }
  }
}

void TLSCredProcessor::certFileUpdated() noexcept {
  for (const auto& callback : certCallbacks_.copy()) {
    (*callback)();
  }
}

/* static */ Optional<TLSTicketKeySeeds> TLSCredProcessor::processTLSTickets(
    const std::string& fileName,
    const folly::Optional<std::string>& password) {
  try {
    std::string jsonData;
    if (password.has_value()) {
      auto wrappedData = SSLUtil::decryptOpenSSLEncFilePassString(
          fileName, password.value(), EVP_aes_256_cbc(), EVP_sha256());
      if (wrappedData.has_value()) {
        jsonData = wrappedData.value();
      } else {
        LOG(WARNING) << "Failed to read " << fileName
                     << " using supplied password "
                     << "; Ticket seeds are unavailable.";
        return folly::none;
      }
    } else {
      if (!folly::readFile(fileName.c_str(), jsonData)) {
        LOG(WARNING) << "Failed to read " << fileName
                     << "; Ticket seeds are unavailable.";
        return folly::none;
      }
    }

    folly::dynamic conf = folly::parseJson(jsonData);
    if (conf.type() != dynamic::Type::OBJECT) {
      LOG(WARNING) << "Error parsing " << fileName << " expected object";
      return folly::none;
    }
    TLSTicketKeySeeds seedData;
    if (conf.count("old")) {
      insertSeeds(conf["old"], seedData.oldSeeds);
    }
    if (conf.count("current")) {
      insertSeeds(conf["current"], seedData.currentSeeds);
    }
    if (conf.count("new")) {
      insertSeeds(conf["new"], seedData.newSeeds);
    }
    return seedData;
  } catch (const std::exception&) {
    // Don't log ex.what() since it may contain contents of the key file.
    LOG(WARNING) << "Parsing " << fileName << " failed.";
    return folly::none;
  }
}

} // namespace wangle
