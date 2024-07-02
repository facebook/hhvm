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

#include <string>

// The use of boost::regex results in CLANGTIDY warnings.
// https://fburl.com/cxxRegexes describes performance issues in production
// with boost::regex as well as std::regex. The thrift compiler
// does not run in production. Therefore, we can safely ignore these warnings
// with @lint-ignore CLANGTIDY facebook-... (see below)

#define BOOST_REGEX_NO_W32
// @lint-ignore CLANGTIDY facebook-hte-BadInclude-regex
#include <boost/regex.hpp>

namespace apache::thrift::compiler {

inline bool is_reserved_identifier_name(std::string_view name) {
  // @lint-ignore CLANGTIDY facebook-hte-BoostRegexRisky
  static const boost::regex reserved_pattern(
      "^_*fbthrift", boost::regex::icase);
  return boost::regex_search(
      std::begin(name),
      std::end(name),
      // @lint-ignore CLANGTIDY facebook-hte-BoostRegexRisky
      reserved_pattern);
}

} // namespace apache::thrift::compiler
