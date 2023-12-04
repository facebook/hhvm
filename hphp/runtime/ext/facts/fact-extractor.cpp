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
#include <folly/logging/xlog.h>

#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/facts/exception.h"
#include "hphp/runtime/ext/facts/fact-extractor.h"
#include "hphp/runtime/ext/facts/thread-factory.h"
#include "hphp/runtime/vm/unit-parser.h"
#include "hphp/util/logger.h"
#include "hphp/util/match.h"
#include "hphp/util/text-util.h"

namespace HPHP {
namespace Facts {

namespace {

std::vector<std::string> move_str_vec(rust::Vec<rust::String> stringList) {
  std::vector<std::string> ret;
  ret.reserve(stringList.size());
  for (auto& item : std::move(stringList)) {
    ret.emplace_back(std::move(item));
  }
  return ret;
}

std::vector<Attribute> move_attr_vec(rust::Vec<hackc::AttrFacts> attrList) {
  std::vector<Attribute> ret;
  ret.reserve(attrList.size());
  for (auto& item : std::move(attrList)) {
    Attribute attr;
    attr.m_name = std::string{std::move(item.name)};
    for (auto& arg : item.args) {
      attr.m_args.emplace_back(std::move(arg));
    }
    ret.emplace_back(std::move(attr));
  }
  return ret;
}

std::vector<MethodDetails> move_method_vec(
    rust::Vec<hackc::MethodFacts> methodList) {
  std::vector<MethodDetails> ret;
  ret.reserve(methodList.size());
  for (auto& method : std::move(methodList)) {
    ret.push_back(MethodDetails{
        .m_name = std::string{method.name},
        .m_attributes = move_attr_vec(std::move(method.details.attributes))});
  }
  return ret;
}

std::vector<TypeDetails> move_type_vec(rust::Vec<hackc::TypeFacts> types) {
  std::vector<TypeDetails> ret;
  for (auto& type : std::move(types)) {
    auto typeKind = [&] {
      switch (type.kind) {
        case hackc::TypeKind::Class:
          return TypeKind::Class;
        case hackc::TypeKind::Interface:
          return TypeKind::Interface;
        case hackc::TypeKind::Enum:
          return TypeKind::Enum;
        case hackc::TypeKind::Trait:
          return TypeKind::Trait;
        case hackc::TypeKind::TypeAlias:
          return TypeKind::TypeAlias;
        case hackc::TypeKind::Mixed:
        case hackc::TypeKind::Unknown:
        default:
          return TypeKind::Unknown;
      }
    }();
    ret.push_back(TypeDetails{
        .m_name = std::string{type.name},
        .m_kind = typeKind,
        .m_flags = type.flags,
        .m_baseTypes = move_str_vec(std::move(type.base_types)),
        .m_attributes = move_attr_vec(std::move(type.attributes)),
        .m_requireClass = move_str_vec(std::move(type.require_class)),
        .m_requireExtends = move_str_vec(std::move(type.require_extends)),
        .m_requireImplements = move_str_vec(std::move(type.require_implements)),
        .m_methods = move_method_vec(std::move(type.methods))});
  }
  return ret;
}

std::vector<ModuleDetails> move_module_vec(
    rust::Vec<hackc::ModuleFacts> modules) {
  std::vector<ModuleDetails> ret;
  ret.reserve(modules.size());
  for (auto& module : std::move(modules)) {
    ret.push_back(ModuleDetails{.m_name = std::string{module.name}});
  }
  return ret;
}

FileFacts make_file_facts(hackc::FileFacts facts) {
  try {
    return {
        .m_types = move_type_vec(std::move(facts.types)),
        .m_functions = move_str_vec(std::move(facts.functions)),
        .m_constants = move_str_vec(std::move(facts.constants)),
        .m_modules = move_module_vec(std::move(facts.modules)),
        .m_attributes = move_attr_vec(std::move(facts.file_attributes)),
        .m_sha1hex = std::string{facts.sha1sum}};
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

hackc::FileFacts parse_json(const std::string& json) {
  try {
    return hackc::json_to_facts(json);
  } catch (const std::exception& e) {
    throw FactsExtractionExc{folly::sformat(
        "{} - JSON is \"{}\"", e.what(), summarized_string(json, 80))};
  }
}

ExtractorFactory* s_extractorFactory = nullptr;

struct SimpleExtractor final : Extractor {
  explicit SimpleExtractor(folly::Executor& exec) : Extractor{exec} {}

  ~SimpleExtractor() override = default;

  folly::SemiFuture<std::string> get(const PathAndOptionalHash& key) override {
    return folly::via(&m_exec, [key]() { return facts_json_from_path(key); });
  }
};

} // namespace

std::string facts_json_from_path(const PathAndOptionalHash& path) {
  assertx(path.m_path.is_absolute());

  auto const result = extract_facts(
      path.m_path.native(),
      RepoOptions::forFile(path.m_path.c_str()).flags(),
      path.m_hash ? *path.m_hash : "");
  return match<std::string>(
      result,
      [&](const FactsJSONString& r) { return r.value; },
      [&](const std::string& err) -> std::string {
        throw FactsExtractionExc{err};
      });
}

void setExtractorFactory(ExtractorFactory* factory) {
  s_extractorFactory = factory;
}

std::unique_ptr<Extractor> makeExtractor(folly::Executor& exec) {
  // If we defined an external Extractor in closed-source code, use that.
  // Otherwise use the SimpleExtractor.
  if (s_extractorFactory && RuntimeOption::AutoloadEnableExternFactExtractor) {
    XLOG(INFO) << "Creating a external HPHP::Facts::Extractor.";
    return s_extractorFactory->make(exec);
  }
  XLOG(INFO) << "Creating an internal HPHP::Facts::SimpleExtractor.";
  return std::make_unique<SimpleExtractor>(exec);
}

std::vector<folly::Try<FileFacts>> facts_from_paths(
    const std::filesystem::path& root,
    const std::vector<PathAndOptionalHash>& pathsAndHashes) {
  folly::CPUThreadPoolExecutor exec{
      std::min(
          RuntimeOption::EvalFactsWorkers,
          static_cast<uint64_t>(pathsAndHashes.size())),
      make_thread_factory("FactExtractor")};

  // If we defined an external Extractor in closed-source code, use that.
  // Otherwise use the SimpleExtractor.
  auto extractor = makeExtractor(exec);

  std::atomic<int> completed_tasks = 0;
  std::vector<folly::SemiFuture<FileFacts>> factsFutures;
  factsFutures.reserve(pathsAndHashes.size());

  XLOGF(INFO, "Extracting facts for {} files.", pathsAndHashes.size());
  for (int i = 0; i < pathsAndHashes.size(); ++i) {
    auto const& pathAndHash = pathsAndHashes.at(i);
    XLOG_EVERY_N(INFO, 50000) << "Enqueued " << i << " out of "
                              << pathsAndHashes.size() << " updates.";

    assertx(pathAndHash.m_path.is_relative());
    PathAndOptionalHash absPathAndHash{
        root / pathAndHash.m_path, pathAndHash.m_hash};
    factsFutures.push_back(
        folly::via(
            &exec,
            [&extractor, absPathAndHash]() {
              if (UNLIKELY(!absPathAndHash.m_hash)) {
                // We don't know the file's hash yet, so we don't know
                // which key to use to query memcache. We'll try to extract
                // facts from disk instead.
                throw FactsExtractionExc{"No hash provided"};
              }
              return extractor->get(absPathAndHash);
            })
            .thenValue(
                [absPathAndHash](std::string&& factsJson) -> hackc::FileFacts {
                  auto facts = parse_json(factsJson);
                  auto const& hash = *absPathAndHash.m_hash;
                  if (UNLIKELY(facts.sha1sum != hash)) {
                    // The hash we got out of memcache doesn't match the hash
                    // we expected. We'll try to extract facts from disk
                    // instead.
                    throw FactsExtractionExc{folly::sformat(
                        "Error extracting {} from memcache: hash '{}' != '{}'",
                        absPathAndHash.m_path.native(),
                        std::string{facts.sha1sum},
                        hash)};
                  }
                  return facts;
                })
            .thenTry([absPathAndHash](folly::Try<hackc::FileFacts>&& facts) {
              if (facts.hasValue()) {
                return *std::move(facts);
              } else {
                XLOGF(
                    WARN,
                    "Error extracting {}: {}",
                    absPathAndHash.m_path.native().c_str(),
                    facts.exception().what().c_str());
                // There might have been a SHA1 mismatch due to a filesystem
                // race. Try again without an expected hash.
                PathAndOptionalHash withoutHash{absPathAndHash.m_path, {}};
                return parse_json(facts_json_from_path(withoutHash));
              }
            })
            .thenValue([](hackc::FileFacts&& facts) {
              return make_file_facts(std::move(facts));
            })
            .thenTry([&completed_tasks,
                      &pathsAndHashes](folly::Try<FileFacts>&& facts) {
              int completed = ++completed_tasks;
              XLOG_EVERY_N(INFO, 50000)
                  << "Finished " << completed << " out of "
                  << pathsAndHashes.size() << " updates.";
              return std::move(facts);
            }));
  }

  XLOG(INFO) << "Done spawning facts_from_paths futures.";
  return folly::collectAll(factsFutures).wait().get();
}

void prefetchDb(const std::filesystem::path& root, const SQLiteKey& dbKey) {
  XLOG(INFO) << "::prefetchDb " << root << " " << dbKey.toString();
  if (s_extractorFactory && RuntimeOption::AutoloadEnableExternFactExtractor) {
    s_extractorFactory->prefetchDb(root, dbKey);
  }
}

} // namespace Facts
} // namespace HPHP
