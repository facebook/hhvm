/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <algorithm>
#include <memory>
#include <string>

#include <folly/dynamic.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/futures/Future.h>

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/facts/exception.h"
#include "hphp/runtime/ext/facts/fact-extractor.h"
#include "hphp/runtime/ext/facts/thread-factory.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/util/logger.h"
#include "hphp/util/match.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(facts);

namespace HPHP {
namespace Facts {

namespace {

TypeKind fromString(const std::string& str) {
  if (str == "class") {
    return TypeKind::Class;
  } else if (str == "interface") {
    return TypeKind::Interface;
  } else if (str == "enum") {
    return TypeKind::Enum;
  } else if (str == "trait") {
    return TypeKind::Trait;
  } else if (str == "typeAlias") {
    return TypeKind::TypeAlias;
  }
  return TypeKind::Unknown;
}

std::vector<std::string> move_str_vec(folly::dynamic* stringList) {
  if (stringList == nullptr) {
    return {};
  }
  std::vector<std::string> ret;
  ret.reserve(stringList->size());
  for (auto& item : *stringList) {
    ret.push_back(std::move(item).getString());
  }
  return ret;
}

std::vector<Attribute> move_attr_vec(folly::dynamic* attrList) {
  if (attrList == nullptr) {
    return {};
  }

  std::vector<Attribute> ret;
  for (auto& item : attrList->items()) {
    Attribute attr;
    attr.m_name = item.first.getString();
    for (auto& arg : std::move(item.second)) {
      attr.m_args.push_back(std::move(arg));
    }
    ret.push_back(std::move(attr));
  }
  return ret;
}

std::vector<MethodDetails> move_method_vec(folly::dynamic* methodList) {
  if (!methodList) {
    return {};
  }
  std::vector<MethodDetails> ret;
  ret.reserve(methodList->size());
  for (auto& [method, details] : methodList->items()) {
    ret.push_back(MethodDetails{
        .m_name = method.getString(),
        .m_attributes = move_attr_vec(details.get_ptr("attributes"))});
  }
  return ret;
}

std::vector<TypeDetails> move_type_vec(folly::dynamic* types) {
  if (types == nullptr) {
    return {};
  }
  std::vector<TypeDetails> ret;
  for (auto& type : *types) {
    auto typeKind = fromString(std::move(type.at("kindOf")).getString());
    ret.push_back(TypeDetails{
        .m_name = std::move(type.at("name")).getString(),
        .m_kind = typeKind,
        .m_flags = static_cast<int>(std::move(type.at("flags")).getInt()),
        .m_baseTypes = move_str_vec(type.get_ptr("baseTypes")),
        .m_attributes = move_attr_vec(type.get_ptr("attributes")),
        .m_requireExtends = move_str_vec(type.get_ptr("requireExtends")),
        .m_requireImplements = move_str_vec(type.get_ptr("requireImplements")),
        .m_methods = move_method_vec(type.get_ptr("methods"))});
  }
  return ret;
}

FileFacts make_file_facts(folly::dynamic facts) {
  try {
    return {
        .m_types = move_type_vec(facts.get_ptr("types")),
        .m_functions = move_str_vec(facts.get_ptr("functions")),
        .m_constants = move_str_vec(facts.get_ptr("constants")),
        .m_attributes = move_attr_vec(facts.get_ptr("fileAttributes")),
        .m_sha1hex = std::move(facts.at("sha1sum")).getString()};
  } catch (const folly::TypeError& e) {
    throw FactsExtractionExc{e.what()};
  }
}

// Given a string like "foo bla bla bla ... bla bar", returns a
// printable string like "foo [1234 bytes omitted] bar", where the
// length of the prefix and suffix taken from the string are specified
// by `excerpt_len`. Note that the actual output might be a bit
// longer, due to escaping (e.g., if the string starts with nulls).
std::string summarized_string(std::string_view s, int excerpt_len) {
  std::string to_encode;
  // The 20 bytes of slack is to avoid silly things like:
  // [...2 bytes omitted...]
  // where we might as well just print them.
  if (s.size() < 2 * excerpt_len + 20) {
    to_encode = s;
  } else {
    to_encode = folly::sformat(
        "{} [...{} bytes omitted...] {}",
        s.substr(0, excerpt_len),
        s.size() - 2 * excerpt_len,
        s.substr(s.size() - excerpt_len));
  }
  return ::HPHP::escapeStringForCPP(to_encode);
}

folly::dynamic parse_json(const std::string& json) {
  try {
    return folly::parseJson(json);
  } catch (const folly::json::parse_error& e) {
    throw FactsExtractionExc{folly::sformat(
        "{} - JSON is \"{}\"", e.what(), summarized_string(json, 80))};
  }
}

ExtractorFactory s_extractorFactory = nullptr;

struct SimpleExtractor final : public Extractor {
  explicit SimpleExtractor(folly::Executor& exec) : Extractor{exec} {
  }

  ~SimpleExtractor() override = default;

  folly::SemiFuture<std::string> get(const PathAndHash& key) override {
    return folly::via(
        &m_exec, [path = key.m_path]() { return facts_json_from_path(path); });
  }
};

} // namespace

std::string facts_json_from_path(const folly::fs::path& path) {
  assertx(path.is_absolute());
  auto parser = acquire_facts_parser();

  auto const result = extract_facts(
      *parser, path.native(), "", 0, RepoOptions::forFile(path.c_str()));
  return match<std::string>(
      result,
      [&](const FactsJSONString& r) { return r.value; },
      [&](const std::string& err) -> std::string {
        throw FactsExtractionExc{err};
      });
}

void setExtractorFactory(ExtractorFactory factory) {
  s_extractorFactory = factory;
}

std::vector<folly::Try<FileFacts>> facts_from_paths(
    const folly::fs::path& root,
    const std::vector<PathAndHash>& pathsAndHashes) {

  folly::CPUThreadPoolExecutor exec{
      std::min(
          RuntimeOption::EvalHackCompilerWorkers,
          static_cast<uint64_t>(pathsAndHashes.size())),
      make_thread_factory("FactExtractor")};

  // If we defined a fancy memcache Extractor in closed-source code, use that.
  // Otherwise use the SimpleExtractor.
  auto extractor = [&]() -> std::unique_ptr<Extractor> {
    if (s_extractorFactory) {
      FTRACE(3, "Creating a custom HPHP::Facts::Extractor.\n");
      return s_extractorFactory(exec);
    } else {
      FTRACE(3, "Creating a HPHP::Facts::SimpleExtractor.\n");
      return std::make_unique<SimpleExtractor>(exec);
    }
  }();

  std::vector<folly::SemiFuture<FileFacts>> factsFutures;
  factsFutures.reserve(pathsAndHashes.size());
  for (auto const& pathAndHash : pathsAndHashes) {
    assertx(pathAndHash.m_path.is_relative());
    PathAndHash absPathAndHash{root / pathAndHash.m_path, pathAndHash.m_hash};
    auto factsFromCacheFuture =
        [&exec, &extractor, absPathAndHash]() -> folly::Future<folly::dynamic> {
      if (UNLIKELY(!absPathAndHash.m_hash)) {
        // We don't know the file's hash yet, so we don't know which key to use
        // to query memcache. We'll try to extract facts from disk instead.
        throw FactsExtractionExc{"No hash provided"};
      }
      return extractor->get(absPathAndHash)
          .via(&exec)
          .thenValue(
              [absPathAndHash](std::string&& factsJson) -> folly::dynamic {
                auto facts = parse_json(factsJson);
                auto const& hash = *absPathAndHash.m_hash;
                if (UNLIKELY(facts.at("sha1sum").getString() != hash)) {
                  // The hash we got out of memcache doesn't match the hash
                  // we expected. We'll try to extract facts from disk
                  // instead.
                  throw FactsExtractionExc{folly::sformat(
                      "Error extracting {} from memcache: hash '{}' != '{}'",
                      absPathAndHash.m_path.native(),
                      facts.at("sha1sum").getString(),
                      hash)};
                }
                return facts;
              });
    }();
    factsFutures.emplace_back(
        std::move(factsFromCacheFuture)
            .thenTry([absPathAndHash = std::move(absPathAndHash)](
                         folly::Try<folly::dynamic>&& facts) {
              if (facts.hasValue()) {
                return *std::move(facts);
              } else {
                Logger::Info(
                    "Error extracting %s: %s\n",
                    absPathAndHash.m_path.native().c_str(),
                    facts.exception().what().c_str());
                return parse_json(facts_json_from_path(absPathAndHash.m_path));
              }
            })
            .thenValue([](folly::dynamic&& facts) {
              return make_file_facts(std::move(facts));
            }));
  }

  return folly::collectAll(factsFutures).wait().get();
}

} // namespace Facts
} // namespace HPHP
