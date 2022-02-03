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
#include <boost/filesystem/operations.hpp>

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

      auto configurationJson = RO::EvalTaintConfigurationJson;
      if (configurationJson != "") {
        configuration->reset(configurationJson);
      } else {
        configuration->read(RO::EvalTaintConfigurationPath);
      }

      auto outputDirectory = RO::EvalTaintOutputDirectory;
      if (outputDirectory != "") {
        boost::filesystem::create_directories(outputDirectory);
        configuration->outputDirectory = outputDirectory;
      }
    },
    InitFiniNode::When::ProcessInit);

folly::Singleton<Configuration, SingletonTag> kSingleton{};

} // namespace

/* static */ std::shared_ptr<Configuration> Configuration::get() {
  return kSingleton.try_get();
}

void Configuration::reset(const std::string& contents) {
  try {
    auto parsed = folly::parseJson(contents);

    std::vector<std::pair<std::string, Source>> sources;
    for (const auto source : parsed["sources"]) {
      Source value{.index = std::nullopt};
      auto indexElement = source.getDefault("index");
      if (indexElement.isInt()) {
        value.index = indexElement.asInt();
      }
      sources.push_back(std::make_pair(source["name"].asString(), value));
    }
    m_sources.store(std::make_shared<TaintedFunctionSet<Source>>(sources));

    std::vector<std::pair<std::string, Sink>> sinks;
    for (const auto sink : parsed["sinks"]) {
      Sink value{.index = sink["index"].asInt()};
      auto name = sink["name"].asString();
      sinks.push_back(std::make_pair(sink["name"].asString(), value));
    }
    m_sinks.store(std::make_unique<TaintedFunctionSet<Sink>>(sinks));
  } catch (std::exception& exception) {
    throw std::invalid_argument(
        std::string("unable to parse configuration: ") + exception.what());
  }
}

void Configuration::read(const std::string& path) {
  if (path == "") {
    return;
  }

  std::string contents;

  try {
    boost::filesystem::load_string_file(path, contents);
  } catch (std::exception& exception) {
    throw std::invalid_argument(
        "unable to read configuration at `" + path + "`: " + exception.what());
  }

  reset(contents);
}

std::shared_ptr<TaintedFunctionSet<Source>> Configuration::sources() {
  auto sources = m_sources.load();
  if (!sources) {
    return nullptr;
  }
  return sources->clone();
}

std::shared_ptr<TaintedFunctionSet<Sink>> Configuration::sinks() {
  auto sinks = m_sinks.load();
  if (!sinks) {
    return nullptr;
  }
  return sinks->clone();
}

void Configuration::updateCachesAfterRequest(
    std::shared_ptr<TaintedFunctionSet<Source>> sources,
    std::shared_ptr<TaintedFunctionSet<Sink>> sinks) {
  m_sources.store(sources);
  m_sinks.store(sinks);
}

} // namespace taint
} // namespace HPHP

#endif
