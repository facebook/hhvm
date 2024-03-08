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

#include "hphp/runtime/base/configs/server.h"

#include "hphp/util/process.h"
#include "hphp/util/process-cpu.h"

#include <climits>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace HPHP::Cfg {

int Server::ThreadCountDefault() {
  return Process::GetCPUCount() * 2;
}

int Server::WarmupThrottleThreadCountDefault() {
  return Process::GetCPUCount();
}

void Server::CacheFreeFactorPostProcess(int& value) {
  if (value > 100) value = 100;
  if (value < 0) value = 0;
}

void Server::MaxPostSizePostProcess(int64_t& value) {
  value <<= 20;
}

void Server::ExpiresDefaultPostProcess(int& value) {
  if (value < 0) value = 2592000;
}

void Server::SSLClientCAFilePostProcess(std::string& value) {
  if (!SSLClientAuthLevel) {
      value = "";
    } else if (value.empty()) {
      throw std::runtime_error(
          "SSLClientCAFile is required to enable client auth");
    }
}

void Server::SSLClientAuthLevelPostProcess(int& value) {
  if (value < 0) value = 0;
  if (value > 2) value = 2;
}

void Server::ClientAuthLogSampleBasePostProcess(uint32_t& value) {
  if (value < 1) {
    value = 1;
  }
}

void Server::ClientAuthSuccessLogSampleRatioPostProcess(uint32_t& value) {
  if (value > ClientAuthLogSampleBase) {
    value = ClientAuthLogSampleBase;
  }
}

void Server::ClientAuthFailureLogSampleRatioPostProcess(uint32_t& value) {
  if (value > ClientAuthLogSampleBase) {
    value = ClientAuthLogSampleBase;
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

void Server::ErrorDocument404PostProcess(std::string& value) {
  normalizePath(value);
}

void Server::ErrorDocument500PostProcess(std::string& value) {
  normalizePath(value);
}

std::string Server::SourceRootDefault() {
  return Process::GetCurrentDirectory() + '/';
}

void Server::MaxArrayChainPostProcess(int& value) {
  // VanillaDict needs a higher threshold to avoid false-positives.
  // (and we always use VanillaDict)
  if (value < (INT_MAX / 2)) {
    value *= 2;
  } else {
    value = INT_MAX;
  }
}

int Server::ThreadJobLIFOSwitchThresholdDefault() {
  if (ThreadJobLIFO) return 0;
  return INT_MAX;
}

void Server::UploadMaxFileSizePostProcess(int64_t& value) {
  value <<= 20;
}

void Server::UploadRfc1867FreqPostProcess(int& value) {
  if (value < 0) value = 256 * 1024;
}

void Server::ImageMemoryMaxBytesPostProcess(int64_t& value) {
  if (value == 0) {
    value = UploadMaxFileSize * 2;
  }
}

}
