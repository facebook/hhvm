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

#pragma once

#include <set>
#include <string>

#include <folly/Optional.h>
#include <folly/Synchronized.h>
#include <folly/io/async/PasswordInFile.h>
#include <wangle/ssl/TLSTicketKeySeeds.h>
#include <wangle/util/FilePoller.h>

namespace wangle {

/**
 * A class that monitors files related to TLS credentials that fire callbacks
 * when they change.  Callbacks are fired in a background thread.
 */
class TLSCredProcessor {
 public:
  TLSCredProcessor();
  explicit TLSCredProcessor(std::chrono::milliseconds pollInterval);

  virtual ~TLSCredProcessor();

  /**
   * Set the ticket path to watch for file (optionally) encrypted with password.
   * Any previous ticket path will stop being watched.  This is not thread safe.
   */
  virtual void setTicketPathToWatch(
      const std::string& ticketFile,
      const folly::Optional<std::string>& password = folly::none);

  /**
   * Set cert related files to watch.  This would include paths like
   * cert, key, and CA.  Cert callbacks will be fired if any of these
   * change.  Empty strings are ignored.
   */
  virtual void setCertPathsToWatch(std::set<std::string> certFiles);

  virtual void addTicketCallback(
      std::function<void(wangle::TLSTicketKeySeeds)> callback);
  virtual void addCertCallback(std::function<void()> callback);

  void stop();

  virtual void setPollInterval(std::chrono::milliseconds pollInterval);

  /**
   * This parses a TLS ticket file with the tickets and returns a
   * TLSTicketKeySeeds structure if the file is valid.
   * The TLS ticket file is formatted as a json blob
   * {
   *   "old": [
   *     "seed1",
   *     ...
   *   ],
   *   "new": [
   *     ...
   *   ],
   *   "current": [
   *     ...
   *   ]
   * }
   * Seeds are aribitrary length secret strings which are used to derive
   * ticket encryption keys.
   */
  static folly::Optional<wangle::TLSTicketKeySeeds> processTLSTickets(
      const std::string& fileName,
      const folly::Optional<std::string>& password = folly::none);

 private:
  void ticketFileUpdated(
      const std::string& ticketFile,
      const folly::Optional<std::string>& password) noexcept;

  void certFileUpdated() noexcept;

  std::unique_ptr<FilePoller> poller_;
  std::string ticketFile_;
  folly::Optional<std::string> password_;
  std::set<std::string> certFiles_;
  folly::Synchronized<std::vector<
      std::shared_ptr<std::function<void(wangle::TLSTicketKeySeeds)>>>>
      ticketCallbacks_;
  folly::Synchronized<std::vector<std::shared_ptr<std::function<void()>>>>
      certCallbacks_;
};
} // namespace wangle
