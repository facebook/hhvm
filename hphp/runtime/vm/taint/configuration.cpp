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

bool Sink::operator==(const Sink& other) const {
  return name == other.name && index == other.index;
}

namespace {

struct SingletonTag {};

} // namespace

InitFiniNode s_configurationInitialization(
    []() {
      Configuration::get()->read(RO::EvalTaintConfigurationPath);
    },
    InitFiniNode::When::ProcessInit);

void Configuration::read(const std::string& path) {
  sources.clear();
  m_sinks.clear();

  try {
    std::string contents;
    boost::filesystem::load_string_file(path, contents);
    auto parsed = folly::parseJson(contents);
    for (const auto source : parsed["sources"]) {
      sources.insert(source.asString());
    }
    for (const auto sink : parsed["sinks"]) {
      addSink(Sink{sink["name"].asString(), sink["index"].asInt()});
    }
  } catch (std::exception& exception) {
    // Swallow because we don't use it in tests.
    std::cerr << "taint: warning, unable to read configuration ("
              << exception.what() << ")" << std::endl;
  }
}

folly::Singleton<Configuration, SingletonTag> kSingleton{};
/* static */ std::shared_ptr<Configuration> Configuration::get() {
  return kSingleton.try_get();
}

void Configuration::addSink(const Sink& sink) {
  auto sinks = m_sinks.find(sink.name);
  if (sinks != m_sinks.end()) {
      sinks->second.insert(sink);
  }
  m_sinks.insert({sink.name, SinkSet{sink}});
}

static SinkSet kEmptySet = {};

const SinkSet& Configuration::sinks(const std::string& name) const {
  auto sinks = m_sinks.find(name);
  if (sinks != m_sinks.end()) {
    return sinks->second;
  }
  return kEmptySet;
}

} // namespace taint
} // namespace HPHP

#endif
