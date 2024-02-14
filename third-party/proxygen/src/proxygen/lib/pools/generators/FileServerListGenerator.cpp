/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/FileUtil.h>
#include <folly/SocketAddress.h>
#include <folly/dynamic.h>
#include <folly/json.h>
#include <proxygen/lib/pools/generators/FileServerListGenerator.h>
#include <sstream>

using folly::dynamic;
using folly::parseJson;
using folly::SocketAddress;
using std::string;
using std::chrono::milliseconds;

namespace proxygen {

void FileServerListGenerator::FileGenerator::readFile(std::string& filePath,
                                                      std::string& content) {
  if (!folly::readFile(filePath.c_str(), content)) {
    folly::throw_exception<Exception>("Error reading file %s", filePath);
  }
}
void FileServerListGenerator::FileGenerator::cancelServerListRequest() {
}

void FileServerListGenerator::FileGenerator::run(milliseconds /*timeout*/) {
  // Read the file

  VLOG(4) << "Looking up server list from File Handle " << params_->fileName;

  std::string content;
  try {
    readFile(params_->fileName, content);
  } catch (const std::exception&) {
    callback_->serverListError(std::current_exception());
    delete this;
    return;
  }

  // process the content and get the server list
  std::vector<ServerConfig> servers;

  if (params_->fileType == FileType::PLAIN_TEXT) {
    std::stringstream sstream(content);
    string line;
    while (std::getline(sstream, line)) {
      SocketAddress address;
      address.setFromHostPort(line);
      servers.push_back(ServerConfig(address.getAddressStr(), address));
    }
  } else if (params_->fileType == FileType::JSON) {
    try {
      dynamic parsedJson = parseJson(content);
      dynamic poolMembers = parsedJson.getDefault(params_->poolName, -1);
      // If we cannot parse out an arrray out of that.
      if (!poolMembers.isArray()) {
        callback_->serverListError(std::make_exception_ptr(
            std::invalid_argument("Cannot find a valid pool " +
                                  params_->poolName + " in file " +
                                  params_->fileName)));
        delete this;
        return;
      }
      // Now we have an array.
      for (const auto& e : poolMembers) {
        SocketAddress address;
        address.setFromHostPort(e.asString());
        servers.emplace_back(address.getAddressStr(), address);
      }
    } catch (const std::exception&) {
      callback_->serverListError(std::current_exception());
      delete this;
      return;
    }
  } else {
    // Unsupported FileType yet.
    LOG(FATAL) << "Unsupported FileServerListGenerator::FileType!";
  }

  VLOG(4) << "Found " << servers.size() << " usable servers from File "
          << params_->fileName;
  callback_->serverListAvailable(std::move(servers));
  delete this;
}

FileServerListGenerator::FileServerListGenerator(const string& fileName,
                                                 const FileType fileType,
                                                 const string& poolName,
                                                 const uint16_t port)
    : params_(fileName, fileType, poolName, port) {
  if (fileType == FileType::PLAIN_TEXT && !poolName.empty()) {
    throw std::invalid_argument(
        "FileServerListGenerator cannot accept a non-empty poolName parameter "
        "when FileType is PLAIN_TEXT.");
  }
  if (fileType == FileType::JSON && poolName.empty()) {
    throw std::invalid_argument(
        "FileServerListGenerator cannot accept an empty poolName parameter "
        "when FileType is JSON.");
  }
}

void FileServerListGenerator::listServers(Callback* callback,
                                          milliseconds timeout) {
  auto gen = new FileGenerator(&params_, callback);
  callback->resetGenerator(gen);
  gen->run(timeout);
}

} // namespace proxygen
