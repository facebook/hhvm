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

#include "hphp/util/multiglob.h"

#include <folly/String.h>

#include <fnmatch.h>
#include <ranges>

namespace HPHP {

namespace {

struct Matcher;

struct Matcher {

  Matcher() = default;
  Matcher(const Matcher& other) = default;
  // @lint-ignore CLANGTIDY clang-diagnostic-unused-member-function (complains if I add the method and complains if I don't)
  Matcher(Matcher&& other) = default;
  Matcher& operator=(const Matcher& other) = default;
  // @lint-ignore CLANGTIDY clang-diagnostic-unused-member-function (complains if I add the method and complains if I don't)
  Matcher& operator=(Matcher&& other) = default;
  Matcher(std::shared_ptr<Matcher> recursiveWildCardMatcher,
          std::shared_ptr<Matcher> wildCardMatcher,
          std::unordered_map<std::string, std::shared_ptr<Matcher>> staticMatchers,
          std::vector<std::pair<std::string, std::shared_ptr<Matcher>>> patternMatchers,
          bool isDone)
  : recursiveWildCardMatcher(std::move(recursiveWildCardMatcher)),
    wildCardMatcher(std::move(wildCardMatcher)),
    staticMatchers(std::move(staticMatchers)),
    patternMatchers(std::move(patternMatchers)),
    isDone(isDone) {}

  ~Matcher() = default;

  static std::shared_ptr<Matcher> fromString(const std::string& pattern);

  static std::shared_ptr<Matcher> merge(const std::vector<std::shared_ptr<Matcher>>& matchers,
                                        const std::shared_ptr<Matcher>& recursiveWildCardMatcher = nullptr);

  void matches(const std::filesystem::path& root, std::vector<std::filesystem::path>& res) const;

  std::string toString(size_t indent = 0) const;

private:
  std::shared_ptr<Matcher> recursiveWildCardMatcher = {};
  std::shared_ptr<Matcher> wildCardMatcher = {};
  std::unordered_map<std::string, std::shared_ptr<Matcher>> staticMatchers = {};
  std::vector<std::pair<std::string, std::shared_ptr<Matcher>>> patternMatchers = {};
  bool isDone = false;
};

// @lint-ignore CLANGTIDY clang-diagnostic-unused-member-function (used for debugging)
std::string Matcher::toString(size_t indent) const {
  auto res = std::string();
  res += "Matcher: (" + std::to_string(reinterpret_cast<uintptr_t>(this)) + ")\n";
  if (recursiveWildCardMatcher) {
    res += "recursiveWildCardMatcher\n";
    res += recursiveWildCardMatcher->toString(indent + 2);
  }
  if (wildCardMatcher) {
    res += "wildCardMatcher\n";
    res += wildCardMatcher->toString(indent + 2);
  }
  if (!staticMatchers.empty()) {
    res += "staticMatchers:\n";
    for (const auto& [key, value] : staticMatchers) {
      res += "  " + key + "\n" + value->toString(indent + 4);
    }
  }
  if (!patternMatchers.empty()) {
    res += "patternMatchers:\n";
    for (const auto& [key, value] : patternMatchers) {
      res += "  " + key + "\n" + value->toString(indent + 4);
    }
  }
  res += "isDone: " + std::to_string(isDone);

  if (indent > 0) {
    std::vector<std::string_view> lines;
    folly::split('\n', res, lines);
    std::string indent_res;
    for (auto& line : lines) {
      indent_res += std::string(indent, ' ');
      indent_res += line;
      indent_res += "\n";
    }

    res = indent_res;
  }

  return res + "\n";
}

std::shared_ptr<Matcher> Matcher::fromString(const std::string& pattern) {
  auto matcher = std::make_shared<Matcher>();
  matcher->isDone = true;

  std::vector<std::string_view> parts;
  folly::split('/', pattern, parts);

  static const std::string PATTERN_CHARS = "*?[]";

  for (auto& part : std::ranges::views::reverse(parts)) {
    auto matcher_next = std::make_shared<Matcher>();
    if (part == "*") {
      matcher_next->wildCardMatcher = matcher;
    } else if (part == "**") {
      // If we find /**/**/ we can need to remove one because it doesn't affect anything and it makes matching harder
      if (matcher->recursiveWildCardMatcher != nullptr) {
        matcher_next = std::move(matcher);
      } else {
        matcher_next->recursiveWildCardMatcher = matcher;
      }
    } else if (std::find_first_of(part.begin(), part.end(), PATTERN_CHARS.begin(), PATTERN_CHARS.end()) != part.end()) {
      matcher_next->patternMatchers.emplace_back(part, matcher);
    } else {
      matcher_next->staticMatchers[std::string(part)] = matcher;
    }
    matcher = std::move(matcher_next);
  }
  return matcher;
}


std::shared_ptr<Matcher> Matcher::merge(const std::vector<std::shared_ptr<Matcher>>& matchers,
                                        const std::shared_ptr<Matcher>& recursiveWildCardMatcher) {
  if (recursiveWildCardMatcher == nullptr) {
    if (matchers.empty()) return nullptr;
    if (matchers.size() == 1 && matchers[0]->recursiveWildCardMatcher == nullptr) return matchers[0];
  }

  std::vector<std::shared_ptr<Matcher>> recursiveWildCardMatchers;
  std::vector<std::shared_ptr<Matcher>> wildCardMatchers;
  std::unordered_map<std::string, std::vector<std::shared_ptr<Matcher>>> staticMatchers;
  std::unordered_map<std::string, std::vector<std::shared_ptr<Matcher>>> patternMatchers;
  bool isDone = false;

  auto add = [&](std::shared_ptr<Matcher> matcher) {
    for (const auto& [key, value] : matcher->staticMatchers) {
      staticMatchers[key].push_back(value);
    }
    for (const auto& [key, value] : matcher->patternMatchers) {
      patternMatchers[key].push_back(value);
    }
    if (auto& wildCardMatcher = matcher->wildCardMatcher) {
      wildCardMatchers.push_back(wildCardMatcher);
    }
    isDone |= matcher->isDone;
  };


  for (auto& matcher : matchers) {
    add(matcher);

    if (auto& m = matcher->recursiveWildCardMatcher) {
      recursiveWildCardMatchers.push_back(m);
    }
  }
  if (recursiveWildCardMatcher != nullptr) {
    recursiveWildCardMatchers.push_back(recursiveWildCardMatcher);
  }

  std::unordered_map<std::string, std::shared_ptr<Matcher>> staticMatchersMerged;
  staticMatchersMerged.reserve(staticMatchers.size());
  for (const auto& [key, values] : staticMatchers) {
    staticMatchersMerged[key] = Matcher::merge(values);
  }
  std::vector<std::pair<std::string, std::shared_ptr<Matcher>>> patternMatchersMerged;
  patternMatchersMerged.reserve(patternMatchers.size());
  for (const auto& [key, values] : patternMatchers) {
    patternMatchersMerged.emplace_back(key, Matcher::merge(values));
  }

  return std::make_shared<Matcher>(Matcher::merge(recursiveWildCardMatchers),
                                   Matcher::merge(wildCardMatchers),
                                   std::move(staticMatchersMerged),
                                   std::move(patternMatchersMerged),
                                   isDone);
}

void Matcher::matches(const std::filesystem::path& root, std::vector<std::filesystem::path>& res) const {
  if (std::filesystem::is_regular_file(root)) {
    if (isDone || (recursiveWildCardMatcher && recursiveWildCardMatcher->isDone)) {
      res.push_back(root);
    }
    return;
  }

  if (!std::filesystem::is_directory(root)) return;

  if (recursiveWildCardMatcher != nullptr || wildCardMatcher != nullptr || !patternMatchers.empty()) {
    for (auto const& dir_entry : std::filesystem::directory_iterator(root)) {
      auto& newRoot = dir_entry.path();
      auto filename = newRoot.filename().string();

      std::vector<std::shared_ptr<Matcher>> matchers;

      auto checkMatchers = [&](const Matcher* matcher) {
        // Static Matchers
        auto it = matcher->staticMatchers.find(filename);
        if (it != matcher->staticMatchers.end()) {
          matchers.push_back(it->second);
        }

        // Pattern Matchers
        for (const auto& [pattern, m] : matcher->patternMatchers) {
          if (fnmatch(pattern.c_str(), filename.c_str(), 0) == 0) {
            matchers.push_back(m);
          }
        }

        // Wildcard Matcher
        if (matcher->wildCardMatcher != nullptr) {
          matchers.push_back(matcher->wildCardMatcher);
        }
      };

      checkMatchers(this);
      if (recursiveWildCardMatcher != nullptr) {
        checkMatchers(recursiveWildCardMatcher.get());
      }

      auto matcher = Matcher::merge(matchers, recursiveWildCardMatcher);
      if (matcher != nullptr) {
        matcher->matches(newRoot, res);
      }
    }
  } else {
    for (const auto& [key, value] : staticMatchers) {
      auto newRoot = root / key;
      if (!std::filesystem::exists(newRoot)) continue;
      value->matches(newRoot, res);
    }
  }
}

}

std::vector<std::filesystem::path> MultiGlob::matches(const std::set<std::string>& patterns, const std::filesystem::path& root) {
  std::vector<std::shared_ptr<Matcher>> matchers;
  matchers.reserve(patterns.size());
  for (auto& pattern : patterns) {
    matchers.push_back(Matcher::fromString(pattern));
  }

  auto matcher = Matcher::merge(matchers);

  std::vector<std::filesystem::path> res;

  matcher->matches(root, res);
  return res;
}

} // namespace HPHP
