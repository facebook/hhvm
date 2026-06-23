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
#include "hphp/runtime/base/type-variant.h"
#include "hphp/util/sandbox-events.h"

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

void FactsLogger::validate(const std::set<std::string>& types_to_ignore) {
  return logPerf(__func__, "", [&]() { m_inner->validate(types_to_ignore); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeOrTypeAliasFile(
    const OptString& typeName) {
  return logPerf(__func__, typeName.slice(), [&]() {
    return m_inner->getTypeOrTypeAliasFile(typeName);
  });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeOrTypeAliasFileRelative(
    const OptString& typeName) {
  return logPerf(__func__, typeName.slice(), [&]() {
    return m_inner->getTypeOrTypeAliasFileRelative(typeName);
  });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeFile(
    const OptString& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getTypeFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeFileRelative(
    const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypeFileRelative(name);
  });
}

Optional<AutoloadMap::FileResult> FactsLogger::getFunctionFile(
    const OptString& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getFunctionFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getFunctionFileRelative(
    const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getFunctionFileRelative(name);
  });
}

Optional<AutoloadMap::FileResult> FactsLogger::getConstantFile(
    const OptString& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getConstantFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getConstantFileRelative(
    const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getConstantFileRelative(name);
  });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeAliasFile(
    const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypeAliasFile(name);
  });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeAliasFileRelative(
    const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypeAliasFileRelative(name);
  });
}

Optional<AutoloadMap::FileResult> FactsLogger::getModuleFile(
    const OptString& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getModuleFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getModuleFileRelative(
    const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getModuleFileRelative(name);
  });
}

Optional<std::filesystem::path> FactsLogger::getTypeOrTypeAliasFile(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getTypeOrTypeAliasFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getTypeOrTypeAliasFileRelative(
    std::string_view name) {
  return logPerf(__func__, name, [&]() {
    return m_inner->getTypeOrTypeAliasFileRelative(name);
  });
}

Optional<std::filesystem::path> FactsLogger::getTypeFile(
    std::string_view name) {
  return logPerf(__func__, name, [&]() { return m_inner->getTypeFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getTypeFileRelative(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getTypeFileRelative(name); });
}

Optional<std::filesystem::path> FactsLogger::getFunctionFile(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getFunctionFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getFunctionFileRelative(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getFunctionFileRelative(name); });
}

Optional<std::filesystem::path> FactsLogger::getConstantFile(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getConstantFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getConstantFileRelative(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getConstantFileRelative(name); });
}

Optional<std::filesystem::path> FactsLogger::getTypeAliasFile(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getTypeAliasFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getTypeAliasFileRelative(
    std::string_view name) {
  return logPerf(__func__, name, [&]() {
    return m_inner->getTypeAliasFileRelative(name);
  });
}

Optional<std::string> FactsLogger::getSha1(const OptString& path) {
  return logPerf(
      __func__, path.slice(), [&]() { return m_inner->getSha1(path); });
}

Optional<std::filesystem::path> FactsLogger::getModuleFile(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getModuleFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getModuleFileRelative(
    std::string_view name) {
  return logPerf(
      __func__, name, [&]() { return m_inner->getModuleFileRelative(name); });
}

Array FactsLogger::getFileTypes(const OptString& path) {
  return logPerf(
      __func__, path.slice(), [&]() { return m_inner->getFileTypes(path); });
}

Array FactsLogger::getFileFunctions(const OptString& path) {
  return logPerf(__func__, path.slice(), [&]() {
    return m_inner->getFileFunctions(path);
  });
}

Array FactsLogger::getFileConstants(const OptString& path) {
  return logPerf(__func__, path.slice(), [&]() {
    return m_inner->getFileConstants(path);
  });
}

Array FactsLogger::getFileTypeAliases(const OptString& path) {
  return logPerf(__func__, path.slice(), [&]() {
    return m_inner->getFileTypeAliases(path);
  });
}

Array FactsLogger::getFileModules(const OptString& path) {
  return logPerf(
      __func__, path.slice(), [&]() { return m_inner->getFileModules(path); });
}

Array FactsLogger::getAllModules() {
  return logPerf(__func__, "", [&]() { return m_inner->getAllModules(); });
}

Optional<OptString> FactsLogger::getFileModuleMembership(
    const OptString& path) {
  return logPerf(__func__, path.slice(), [&]() {
    return m_inner->getFileModuleMembership(path);
  });
}

Optional<OptString> FactsLogger::getFilePackageMembership(
    const OptString& path) {
  return logPerf(__func__, path.slice(), [&]() {
    return m_inner->getFilePackageMembership(path);
  });
}

void FactsLogger::close() {
  return logPerf(__func__, "", [&]() { m_inner->close(); });
}

OptString FactsLogger::getTypeName(const OptString& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getTypeName(name); });
}

Variant FactsLogger::getKind(const OptString& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->getKind(name); });
}

bool FactsLogger::isTypeAbstract(const OptString& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->isTypeAbstract(name); });
}

bool FactsLogger::isTypeFinal(const OptString& name) {
  return logPerf(
      __func__, name.slice(), [&]() { return m_inner->isTypeFinal(name); });
}

Array FactsLogger::getBaseTypes(const OptString& name, const Variant& filter) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getBaseTypes(name, filter);
  });
}

Array FactsLogger::getDerivedTypes(
    const OptString& name,
    const Variant& filter) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getDerivedTypes(name, filter);
  });
}

Array FactsLogger::getTransitiveDerivedTypes(
    const OptString& name,
    const Variant& filter,
    bool includeInterfaceRequireExtends) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTransitiveDerivedTypes(
        name, filter, includeInterfaceRequireExtends);
  });
}

Array FactsLogger::getTypesWithAttribute(const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypesWithAttribute(name);
  });
}

Array FactsLogger::getTypeAliasesWithAttribute(const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypeAliasesWithAttribute(name);
  });
}

Array FactsLogger::getMethodsWithAttribute(const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getMethodsWithAttribute(name);
  });
}

Array FactsLogger::getTypeMethodAttributes(const OptString& type) {
  return logPerf(__func__, type.slice(), [&]() {
    return m_inner->getTypeMethodAttributes(type);
  });
}

Array FactsLogger::getFilesWithAttribute(const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getFilesWithAttribute(name);
  });
}

Array FactsLogger::getFilesAndAttrValsWithAttribute(const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getFilesAndAttrValsWithAttribute(name);
  });
}

Array FactsLogger::getFilesWithAttributeAndAnyValue(
    const OptString& name,
    const folly::dynamic& val) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getFilesWithAttributeAndAnyValue(name, val);
  });
}

Array FactsLogger::getTypeAttributes(const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypeAttributes(name);
  });
}

Array FactsLogger::getTypeAliasAttributes(const OptString& name) {
  return logPerf(__func__, name.slice(), [&]() {
    return m_inner->getTypeAliasAttributes(name);
  });
}

Array FactsLogger::getMethodAttributes(
    const OptString& type,
    const OptString& method) {
  return logPerf(__func__, type.slice(), [&]() {
    return m_inner->getMethodAttributes(type, method);
  });
}

Array FactsLogger::getFileAttributes(const OptString& path) {
  return logPerf(__func__, path.slice(), [&]() {
    return m_inner->getFileAttributes(path);
  });
}

Array FactsLogger::getTypeAttrArgs(
    const OptString& type,
    const OptString& attr) {
  return logPerf(__func__, type.slice(), [&]() {
    return m_inner->getTypeAttrArgs(type, attr);
  });
}

Array FactsLogger::getTypeAliasAttrArgs(
    const OptString& type,
    const OptString& attr) {
  return logPerf(__func__, type.slice(), [&]() {
    return m_inner->getTypeAliasAttrArgs(type, attr);
  });
}

Array FactsLogger::getMethodAttrArgs(
    const OptString& type,
    const OptString& method,
    const OptString& attr) {
  return logPerf(__func__, type.slice(), [&]() {
    return m_inner->getMethodAttrArgs(type, method, attr);
  });
}

Array FactsLogger::getFileAttrArgs(
    const OptString& file,
    const OptString& attr) {
  return logPerf(__func__, file.slice(), [&]() {
    return m_inner->getFileAttrArgs(file, attr);
  });
}

} // namespace HPHP::Facts
