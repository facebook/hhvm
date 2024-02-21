// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include <algorithm>
#include <memory>
#include <string>

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <folly/json/dynamic.h>
#include <folly/logging/xlog.h>
#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/decl/decl-extractor.h"
#include "hphp/runtime/ext/decl/exception.h"
#include "hphp/runtime/ext/facts/thread-factory.h"
#include "hphp/runtime/vm/unit-parser.h"

namespace HPHP {
namespace Decl {

namespace {
ExtractorFactory* s_extractorFactory = nullptr;

struct SimpleExtractor final : Extractor {
  explicit SimpleExtractor(folly::Executor& exec) : Extractor{exec} {}

  ~SimpleExtractor() override = default;

  folly::SemiFuture<std::string> get(
      const Facts::PathAndOptionalHash& key) override {
    return folly::via(&m_exec, [key]() {
      auto binaryString = decls_binary_from_path(key);
      return binaryString.value;
    });
  }
};

} // namespace

rust::Box<hackc::DeclsHolder> decode_decls(const std::string& blob) {
  try {
    return hackc::binary_to_decls_holder(blob);
  } catch (const std::exception& e) {
    throw DeclExtractionExc{e.what()};
  }
}

// Returns the content of a file on disk or throws if unreadable.
// Reading the file off disk makes us susceptible to tearing.
// Ideally, we can precompute file + hash and obtain file content via CAS
// but that is, as of now, unimplemented.
std::string readFile(const std::string& filePath) {
  auto w = Stream::getWrapperFromURI(filePath);
  if (!(w && w->isNormalFileStream())) {
    SystemLib::throwRuntimeExceptionObject(
        String("Could not open FileStreamWrapper for ") + filePath);
  }
  const auto f = w->open(filePath, "r", 0, nullptr);
  if (!f) {
    SystemLib::throwRuntimeExceptionObject(
        String("Could not read file: ") + filePath);
  }
  auto const contents = f->read();
  auto const text = contents.toCppString();
  f->close();
  return text;
}

DeclBinaryString decls_binary_from_path(
    const Facts::PathAndOptionalHash& path) {
  assertx(path.m_path.is_absolute());
  try {
    hackc::DeclParserConfig config;
    auto opts = RepoOptions::forFile(path.m_path.c_str());
    opts.flags().initDeclConfig(config);
    config.include_assignment_values = true;
    auto const text = readFile(path.m_path.native());
    auto decls = hackc::parse_decls(
        config, "", {(const uint8_t*)text.data(), (size_t)text.size()});
    auto blob = hackc::decls_holder_to_binary(*decls);
    auto v = std::string{(char*)blob.data(), blob.size()};
    return DeclBinaryString{v};
  } catch (const std::exception&) {
    return DeclBinaryString{};
  }
}

void setExtractorFactory(ExtractorFactory* factory) {
  s_extractorFactory = factory;
}

std::unique_ptr<Extractor> makeExtractor(folly::Executor& exec) {
  // If we defined an external Extractor in closed-source code, use that.
  // Otherwise use the SimpleExtractor.
  if (s_extractorFactory && RuntimeOption::DeclExtensionEnableExternExtractor) {
    // TODO(nzthomas) restore logging without breaking tests
    // XLOG(INFO) << "Creating a external HPHP::Decl::Extractor.";
    return s_extractorFactory->make(exec);
  }
  // XLOG(INFO) << "Creating an internal HPHP::Decl::SimpleExtractor.";
  return std::make_unique<SimpleExtractor>(exec);
}

rust::Box<hackc::DeclsHolder> decl_from_path(
    const std::filesystem::path& root,
    const Facts::PathAndOptionalHash& pathAndHash) {
  folly::CPUThreadPoolExecutor exec{
      1, Facts::make_thread_factory("DeclExtractor")};

  // If we defined an external Extractor in closed-source code, use that.
  // Otherwise use the SimpleExtractor.
  auto extractor = makeExtractor(exec);

  auto path = pathAndHash.m_path.is_relative() ? root / pathAndHash.m_path
                                               : pathAndHash.m_path;
  Facts::PathAndOptionalHash absPathAndHash{path, pathAndHash.m_hash};
  return folly::via(
             &exec,
             [&extractor, absPathAndHash]() {
               if (UNLIKELY(!absPathAndHash.m_hash)) {
                 // We don't know the file's hash yet, so we don't know
                 // which key to use to query memcache. We'll try to extract
                 // facts from disk instead.
                 throw DeclExtractionExc{"No hash provided"};
               }
               return extractor->get(absPathAndHash);
             })
      .thenValue([](std::string&& declBinary) -> rust::Box<hackc::DeclsHolder> {
        auto decl = decode_decls(declBinary);
        // TODO(nzthomas) consider storing sha1sum in ExtDeclFile
        // in order to validate content at time of read
        return decl;
      })
      .thenTry(
          [absPathAndHash](folly::Try<rust::Box<hackc::DeclsHolder>>&& decl) {
            if (decl.hasValue()) {
              return std::move(*decl);
            } else {
              XLOGF(
                  WARN,
                  "Error extracting {}: {}",
                  absPathAndHash.m_path.native().c_str(),
                  decl.exception().what().c_str());
              // // There might have been a SHA1 mismatch due to a filesystem
              // // race. Try again without an expected hash.
              Facts::PathAndOptionalHash withoutHash{absPathAndHash.m_path, {}};
              auto declsBinary = decls_binary_from_path(withoutHash);
              if (declsBinary.value.empty()) {
                throw DeclExtractionExc{
                    "Could not extract decls from disk or cache"};
              }
              auto declsHolder = decode_decls(declsBinary.value);
              return declsHolder;
            }
          })
      .get();
}

} // namespace Decl
} // namespace HPHP
