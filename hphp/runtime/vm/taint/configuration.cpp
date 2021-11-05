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

#ifdef HHVM_TAINT

#include <folly/Singleton.h>
#include <folly/json.h>

#include <boost/filesystem/string_file.hpp>

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/taint/configuration.h"


namespace HPHP {
namespace taint {

namespace {

struct SingletonTag {};

InitFiniNode s_configurationInitialization(
    []() {
      auto configuration = Configuration::get();

      configuration->read(RO::EvalTaintConfigurationPath);

      auto outputDirectory = RO::EvalTaintOutputDirectory;
      if (outputDirectory != "") {
        configuration->outputDirectory = outputDirectory;
      }
    },
    InitFiniNode::When::ProcessInit);

folly::Singleton<Configuration, SingletonTag> kSingleton{};

} // namespace

/* static */ std::shared_ptr<Configuration> Configuration::get() {
  return kSingleton.try_get();
}

void Configuration::read(const std::string& path) {
  m_sources.clear();
  m_sinks.clear();

  if (path == "") {
    return;
  }

  try {
    std::string contents;
    boost::filesystem::load_string_file(path, contents);
    auto parsed = folly::parseJson(contents);
    for (const auto source : parsed["sources"]) {
      Optional<int> index = std::nullopt;
      auto indexElement = source.getDefault("index");
      if (indexElement.isInt()) {
        index = indexElement.asInt();
      }
      m_sources.push_back({
        boost::regex(source["name"].asString()),
        index,
      });
    }
    for (const auto sink : parsed["sinks"]) {
      m_sinks.push_back({
        boost::regex(sink["name"].asString()),
        sink["index"].asInt()
      });
    }
  } catch (std::exception& exception) {
    throw std::invalid_argument(
        "unable to read configuration at `" + path + "`: " + exception.what());
  }
}

std::vector<Source> Configuration::sources(const std::string& name) const {
  std::vector<Source> sources;
  for (auto& source : m_sources) {
    // Naive implementation for now. Lots of room for optimization.
    if (boost::regex_match(name, source.name)) {
      sources.push_back(source);
    }
  }
  return sources;
}

std::vector<Sink> Configuration::sinks(const std::string& name) const {
  std::vector<Sink> sinks;
  for (auto& sink : m_sinks) {
    // Naive implementation similar to matching sources.
    if (boost::regex_match(name, sink.name)) {
      sinks.push_back(sink);
    }
  }
  return sinks;
}

} // namespace taint
} // namespace HPHP

#endif
