/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/configs/server-loader.h"

#include "hphp/util/configs/server.h"
#include "hphp/util/process.h"
#include "hphp/util/process-cpu.h"

#include <climits>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace HPHP::Cfg {

int ServerLoader::ThreadCountDefault() {
  return Process::GetCPUCount() * 2;
}

int ServerLoader::WarmupThrottleThreadCountDefault() {
  return Process::GetCPUCount();
}

void ServerLoader::CacheFreeFactorPostProcess(int& value) {
  if (value > 100) value = 100;
  if (value < 0) value = 0;
}

void ServerLoader::MaxPostSizePostProcess(int64_t& value) {
  value <<= 20;
}

void ServerLoader::ExpiresDefaultPostProcess(int& value) {
  if (value < 0) value = 2592000;
}

void ServerLoader::SSLClientCAFilePostProcess(std::string& value) {
  if (!Cfg::Server::SSLClientAuthLevel) {
      value = "";
    } else if (value.empty()) {
      throw std::runtime_error(
          "SSLClientCAFile is required to enable client auth");
    }
}

void ServerLoader::SSLClientAuthLevelPostProcess(int& value) {
  if (value < 0) value = 0;
  if (value > 2) value = 2;
}

void ServerLoader::ClientAuthLogSampleBasePostProcess(uint32_t& value) {
  if (value < 1) {
    value = 1;
  }
}

void ServerLoader::ClientAuthSuccessLogSampleRatioPostProcess(uint32_t& value) {
  if (value > Cfg::Server::ClientAuthLogSampleBase) {
    value = Cfg::Server::ClientAuthLogSampleBase;
  }
}

void ServerLoader::ClientAuthFailureLogSampleRatioPostProcess(uint32_t& value) {
  if (value > Cfg::Server::ClientAuthLogSampleBase) {
    value = Cfg::Server::ClientAuthLogSampleBase;
  }
}

namespace {

static void normalizePath(std::string& path) {
  if (!path.empty()) {
    if (path[path.length() - 1] == '/') {
      path = path.substr(0, path.length() - 1);
    }
    if (path[0] != '/') {
      path = std::string("/") + path;
    }
  }
}

}

void ServerLoader::ErrorDocument404PostProcess(std::string& value) {
  normalizePath(value);
}

void ServerLoader::ErrorDocument500PostProcess(std::string& value) {
  normalizePath(value);
}

std::string ServerLoader::SourceRootDefault() {
  return Process::GetCurrentDirectory() + '/';
}

void ServerLoader::MaxArrayChainPostProcess(int& value) {
  // VanillaDict needs a higher threshold to avoid false-positives.
  // (and we always use VanillaDict)
  if (value < (INT_MAX / 2)) {
    value *= 2;
  } else {
    value = INT_MAX;
  }
}

int ServerLoader::ThreadJobLIFOSwitchThresholdDefault() {
  if (Cfg::Server::ThreadJobLIFO) return 0;
  return INT_MAX;
}

void ServerLoader::UploadMaxFileSizePostProcess(int64_t& value) {
  value <<= 20;
}

void ServerLoader::UploadRfc1867FreqPostProcess(int& value) {
  if (value < 0) value = 256 * 1024;
}

void ServerLoader::ImageMemoryMaxBytesPostProcess(int64_t& value) {
  if (value == 0) {
    value = Cfg::Server::UploadMaxFileSize * 2;
  }
}

}
