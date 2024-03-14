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
#include "hphp/runtime/ext/facts/log-perf.h"
#include <folly/logging/xlog.h>
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/sandbox-events.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP::Facts {

FactsLogger::FactsLogger(
    std::shared_ptr<FactsStore> inner,
    std::string_view impl,
    uint32_t sampleRate,
    uint32_t errorSampleRate)
    : m_inner{std::move(inner)},
      m_impl{impl},
      m_sampleRate{sampleRate},
      m_errorSampleRate{errorSampleRate} {}

std::shared_ptr<FactsStore> FactsLogger::wrap(
    std::shared_ptr<FactsStore> inner,
    std::string_view impl,
    uint32_t sampleRate,
    uint32_t errorSampleRate) {
  if (sampleRate != 0) {
    XLOGF(DBG0, "FactsLogger enabled for {}", impl);
    return std::make_shared<FactsLogger>(
        std::move(inner), impl, sampleRate, errorSampleRate);
  } else {
    return inner;
  }
}

AutoloadMap::Holder FactsLogger::getNativeHolder() noexcept {
  return Holder{this, [sptr = shared_from_this()]() mutable { sptr.reset(); }};
}

template <typename F>
auto FactsLogger::logPerf(
    std::string_view method,
    std::string_view key,
    F&& func) const {
  return recordSboxEventWithTimingAndError(
      m_sampleRate,
      m_errorSampleRate,
      m_impl,
      method,
      key,
      std::forward<F>(func));
}

void FactsLogger::ensureUpdated() {
  return logPerf(__func__, "", [&]() { m_inner->ensureUpdated(); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeOrTypeAliasFile(
    const String& typeName) {
  return logPerf(__func__, typeName.slice(), [&]() {
    return m_inner->getTypeOrTypeAliasFile(typeName);
  });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeFile(const String& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getTypeFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getFunctionFile(
    const String& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getFunctionFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getConstantFile(
    const String& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getConstantFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeAliasFile(
    const String& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypeAliasFile(name);
  });
}

Optional<AutoloadMap::FileResult> FactsLogger::getModuleFile(
    const String& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getModuleFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getTypeOrTypeAliasFile(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getTypeOrTypeAliasFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getTypeFile(
    std::string_view name) {
  return logPerf(__func__, name, [&]() { return m_inner->getTypeFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getFunctionFile(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getFunctionFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getConstantFile(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getConstantFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getTypeAliasFile(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getTypeAliasFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getModuleFile(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getModuleFile(name); });
}

Array FactsLogger::getFileTypes(const String& path) {
  return logPerf(
      __func__, path.slice(), [&]() { return m_inner->getFileTypes(path); });
}

Array FactsLogger::getFileFunctions(const String& path) {
  return logPerf(__func__, path.slice(), [&]() {
    return m_inner->getFileFunctions(path);
  });
}

Array FactsLogger::getFileConstants(const String& path) {
  return logPerf(__func__, path.slice(), [&]() {
    return m_inner->getFileConstants(path);
  });
}

Array FactsLogger::getFileTypeAliases(const String& path) {
  return logPerf(__func__, path.slice(), [&]() {
    return m_inner->getFileTypeAliases(path);
  });
}

Array FactsLogger::getFileModules(const String& path) {
  return logPerf(
      __func__, path.slice(), [&]() { return m_inner->getFileModules(path); });
}

void FactsLogger::close() {
  return logPerf(__func__, "", [&]() { m_inner->close(); });
}

Variant FactsLogger::getTypeName(const String& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getTypeName(name); });
}

Variant FactsLogger::getKind(const String& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getKind(name); });
}

bool FactsLogger::isTypeAbstract(const String& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->isTypeAbstract(name); });
}

bool FactsLogger::isTypeFinal(const String& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->isTypeFinal(name); });
}

Array FactsLogger::getBaseTypes(const String& name, const Variant& filter) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getBaseTypes(name, filter);
  });
}

Array FactsLogger::getDerivedTypes(const String& name, const Variant& filter) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getDerivedTypes(name, filter);
  });
}

Array FactsLogger::getTypesWithAttribute(const String& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypesWithAttribute(name);
  });
}

Array FactsLogger::getTypeAliasesWithAttribute(const String& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypeAliasesWithAttribute(name);
  });
}

Array FactsLogger::getMethodsWithAttribute(const String& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getMethodsWithAttribute(name);
  });
}

Array FactsLogger::getFilesWithAttribute(const String& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getFilesWithAttribute(name);
  });
}

Array FactsLogger::getFilesAndAttrValsWithAttribute(const String& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getFilesAndAttrValsWithAttribute(name);
  });
}

Array FactsLogger::getFilesWithAttributeAndAnyValue(
    const String& name,
    const folly::dynamic& val) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getFilesWithAttributeAndAnyValue(name, val);
  });
}

Array FactsLogger::getTypeAttributes(const String& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypeAttributes(name);
  });
}

Array FactsLogger::getTypeAliasAttributes(const String& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypeAliasAttributes(name);
  });
}

Array FactsLogger::getMethodAttributes(
    const String& type,
    const String& method) {
  return logPerf(__func__, type.slice(), [&]() {
    return m_inner->getMethodAttributes(type, method);
  });
}

Array FactsLogger::getFileAttributes(const String& path) {
  return logPerf(__func__, path.slice(), [&]() {
    return m_inner->getFileAttributes(path);
  });
}

Array FactsLogger::getTypeAttrArgs(const String& type, const String& attr) {
  return logPerf(__func__, type.slice(), [&]() {
    return m_inner->getTypeAttrArgs(type, attr);
  });
}

Array FactsLogger::getTypeAliasAttrArgs(
    const String& type,
    const String& attr) {
  return logPerf(__func__, type.slice(), [&]() {
    return m_inner->getTypeAliasAttrArgs(type, attr);
  });
}

Array FactsLogger::getMethodAttrArgs(
    const String& type,
    const String& method,
    const String& attr) {
  return logPerf(__func__, type.slice(), [&]() {
    return m_inner->getMethodAttrArgs(type, method, attr);
  });
}

Array FactsLogger::getFileAttrArgs(const String& file, const String& attr) {
  return logPerf(__func__, file.slice(), [&]() {
    return m_inner->getFileAttrArgs(file, attr);
  });
}

} // namespace HPHP::Facts
