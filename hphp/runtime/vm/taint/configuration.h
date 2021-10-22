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

#pragma once

#ifdef HHVM_TAINT

#include <set>
#include <string>
#include <unordered_set>

#include <boost/functional/hash.hpp>
#include <boost/regex.hpp>

namespace HPHP {
namespace taint {

struct Sink {
  boost::regex name;
  int64_t index;
};

struct Configuration {
  static std::shared_ptr<Configuration> get();

  void read(const std::string& path);

  bool isSource(const std::string& name) const;

  void addSink(const Sink& sink);
  std::vector<Sink> sinks(const std::string& name) const;

  Optional<std::string> outputDirectory;
 private:
  std::set<boost::regex> m_sources;
  std::vector<Sink> m_sinks;
};

} // namespace taint
} // namespace HPHP

#endif
