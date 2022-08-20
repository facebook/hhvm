/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/pools/generators/ServerListGenerator.h>
#include <proxygen/lib/utils/Exception.h>
#include <sys/stat.h>

namespace proxygen {

/*
 * A ServerListGenerator implementation that gets the server list from
 * a file.
 */
class FileServerListGenerator : public ServerListGenerator {
 public:
  enum class FileType { PLAIN_TEXT, JSON };
  // If FileType is specified as PLAIN_TEXT, we will read the file line by line.
  // If FileType is specified as JSON, we will parse it as a json and take the
  // entry that's named poolName.
  explicit FileServerListGenerator(
      const std::string& fileName,
      const FileType fileType = FileType::PLAIN_TEXT,
      const std::string& poolName = "");

  void listServers(Callback* callback,
                   std::chrono::milliseconds timeout) override;

 protected:
  struct Params {
    explicit Params(const std::string& file,
                    const FileType fileType,
                    const std::string& pool)
        : fileName(file), fileType(fileType), poolName(pool) {
    }
    std::string fileName;
    FileType fileType;
    std::string poolName;
  };

  class FileGenerator : public ServerListGenerator::Generator {
   public:
    FileGenerator(Params* params, Callback* callback)
        : params_(params), callback_(callback) {
    }
    virtual ~FileGenerator() override {
    }
    virtual void readFile(std::string& filePath, std::string& content);
    virtual void run(std::chrono::milliseconds ms);
    virtual void cancelServerListRequest() override;

   protected:
    Params* params_;
    Callback* callback_;
  };

  Params params_;

 private:
  // Forbidden copy constructor and assignment operator
  FileServerListGenerator(FileServerListGenerator const&) = delete;
  FileServerListGenerator& operator=(FileServerListGenerator const&) = delete;
};

} // namespace proxygen
