#include <algorithm>
#include <memory>
#include <string>

#include <folly/executors/IOThreadPoolExecutor.h>
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
  explicit SimpleExtractor(folly::Executor::KeepAlive<folly::Executor> exec)
      : Extractor{exec} {}

  ~SimpleExtractor() override = default;

  folly::SemiFuture<std::string> get(
      const Facts::PathAndOptionalHash& key) override {
    return folly::via(m_exec, [key]() {
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
    throw DeclExtractionExc{"Could not open FileStreamWrapper for " + filePath};
  }
  const auto f = w->open(filePath, "r", 0, nullptr);
  if (!f) {
    throw DeclExtractionExc{"Could not read file: " + filePath};
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
    auto opts = RepoOptions::forFile(path.m_path.string());
    opts.flags().initDeclConfig(config);
    config.include_assignment_values = true;
    auto const text = readFile(path.m_path.string());
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

std::unique_ptr<Extractor> makeExtractor(
    const folly::Executor::KeepAlive<folly::Executor>& exec,
    bool enableExternExtractor) {
  // If we defined an external Extractor in closed-source code, use that.
  // Otherwise use the SimpleExtractor.
  if (s_extractorFactory && enableExternExtractor) {
    // TODO(nzthomas) restore logging without breaking tests
    // XLOG(INFO) << "Creating a external HPHP::Decl::Extractor.";
    return s_extractorFactory->make(exec);
  }
  // XLOG(INFO) << "Creating an internal HPHP::Decl::SimpleExtractor.";
  return std::make_unique<SimpleExtractor>(exec);
}

/*
 * Returns decls from the supplied path.
 * Schedules and immediately invokes a folly SemiFuture.
 * pathAndHash must supply an absolute path.
 */
rust::Box<hackc::DeclsHolder> decl_from_path(
    const Facts::PathAndOptionalHash& pathAndHash,
    const folly::Executor::KeepAlive<folly::Executor>& exec,
    bool enableExternExtractor) {
  auto semiFuture =
      decl_from_path_async(pathAndHash, exec, enableExternExtractor);
  return std::move(semiFuture).get();
}

/*
 * Returns decls from the supplied path.
 * Schedules and returns a folly SemiFuture.
 * pathAndHash must supply an absolute path.
 */
folly::SemiFuture<rust::Box<hackc::DeclsHolder>> decl_from_path_async(
    const Facts::PathAndOptionalHash& pathAndHash,
    const folly::Executor::KeepAlive<folly::Executor>& exec,
    bool enableExternExtractor) {
  // If we defined an external Extractor in closed-source code, use that.
  // Otherwise use the SimpleExtractor.
  if (!pathAndHash.m_path.is_absolute()) {
    throw DeclExtractionExc{
        "Path must be absolute, got " + pathAndHash.m_path.string()};
  };
  auto extractor = makeExtractor(exec, enableExternExtractor);

  return folly::via(
             exec,
             [extractor = std::move(extractor), pathAndHash]() {
               if (UNLIKELY(!pathAndHash.m_hash)) {
                 // We don't know the file's hash yet, so we don't know
                 // which key to use to query memcache. We'll try to extract
                 // facts from disk instead.
                 throw DeclExtractionExc{"No hash provided"};
               }
               return extractor->get(pathAndHash);
             })
      .thenValue([](std::string&& declBinary) -> rust::Box<hackc::DeclsHolder> {
        auto decl = decode_decls(declBinary);
        // TODO(nzthomas) consider storing sha1sum in ExtDeclFile
        // in order to validate content at time of read
        return decl;
      })
      .thenTry([pathAndHash](folly::Try<rust::Box<hackc::DeclsHolder>>&& decl) {
        if (decl.hasValue()) {
          return std::move(*decl);
        } else {
          XLOGF(
              WARN,
              "Error extracting {}: {}",
              pathAndHash.m_path.native().c_str(),
              decl.exception().what().c_str());
          // // There might have been a SHA1 mismatch due to a filesystem
          // // race. Try again without an expected hash.
          Facts::PathAndOptionalHash withoutHash{pathAndHash.m_path, {}};
          auto declsBinary = decls_binary_from_path(withoutHash);
          if (declsBinary.value.empty()) {
            throw DeclExtractionExc{
                "Could not extract decls from disk or cache"};
          }
          auto declsHolder = decode_decls(declsBinary.value);
          return declsHolder;
        }
      });
}

} // namespace Decl
} // namespace HPHP
