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
#include "hphp/runtime/base/type-variant.h"

namespace HPHP::Facts {
namespace {

template <typename F>
auto logPerf(std::string_view name, F&& func) {
  using namespace std::chrono_literals;
  auto t0 = std::chrono::steady_clock::now();
  SCOPE_EXIT {
    auto tf = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration<double, std::chrono::milliseconds::period>{
            tf - t0};
    if (elapsed > 500ms) {
      XLOGF(DBG7, "[SLOW] {} completed in {:.2} ms", name, elapsed.count());
    } else {
      XLOGF(DBG8, "{} completed in {:.2} ms", name, elapsed.count());
    }
  };
  return func();
}

} // namespace

FactsLogger::FactsLogger(std::shared_ptr<FactsStore> inner)
    : m_inner(std::move(inner)) {}

std::shared_ptr<FactsStore> FactsLogger::wrap(
    std::shared_ptr<FactsStore> inner) {
  if (XLOG_IS_ON(DBG7) || XLOG_IS_ON(DBG8)) {
    XLOGF(INFO, "FactsLogger enabled");
    return std::make_shared<FactsLogger>(std::move(inner));
  } else {
    XLOGF(INFO, "FactsLogger disabled");
    return inner;
  }
}

AutoloadMap::Holder FactsLogger::getNativeHolder() noexcept {
  return Holder{this, [sptr = shared_from_this()]() mutable { sptr.reset(); }};
}

void FactsLogger::ensureUpdated() {
  return logPerf(__func__, [&]() { m_inner->ensureUpdated(); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeOrTypeAliasFile(
    const String& typeName) {
  return logPerf(
      __func__, [&]() { return m_inner->getTypeOrTypeAliasFile(typeName); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeFile(const String& name) {
  return logPerf(__func__, [&]() { return m_inner->getTypeFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getFunctionFile(
    const String& name) {
  return logPerf(__func__, [&]() { return m_inner->getFunctionFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getConstantFile(
    const String& name) {
  return logPerf(__func__, [&]() { return m_inner->getConstantFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getTypeAliasFile(
    const String& name) {
  return logPerf(__func__, [&]() { return m_inner->getTypeAliasFile(name); });
}

Optional<AutoloadMap::FileResult> FactsLogger::getModuleFile(
    const String& name) {
  return logPerf(__func__, [&]() { return m_inner->getModuleFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getTypeOrTypeAliasFile(
    std::string_view name) {
  return logPerf(
      __func__, [&]() { return m_inner->getTypeOrTypeAliasFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getTypeFile(
    std::string_view name) {
  return logPerf(__func__, [&]() { return m_inner->getTypeFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getFunctionFile(
    std::string_view name) {
  return logPerf(__func__, [&]() { return m_inner->getFunctionFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getConstantFile(
    std::string_view name) {
  return logPerf(__func__, [&]() { return m_inner->getConstantFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getTypeAliasFile(
    std::string_view name) {
  return logPerf(__func__, [&]() { return m_inner->getTypeAliasFile(name); });
}

Optional<std::filesystem::path> FactsLogger::getModuleFile(
    std::string_view name) {
  return logPerf(__func__, [&]() { return m_inner->getModuleFile(name); });
}

Array FactsLogger::getFileTypes(const String& path) {
  return logPerf(__func__, [&]() { return m_inner->getFileTypes(path); });
}

Array FactsLogger::getFileFunctions(const String& path) {
  return logPerf(__func__, [&]() { return m_inner->getFileFunctions(path); });
}

Array FactsLogger::getFileConstants(const String& path) {
  return logPerf(__func__, [&]() { return m_inner->getFileConstants(path); });
}

Array FactsLogger::getFileTypeAliases(const String& path) {
  return logPerf(__func__, [&]() { return m_inner->getFileTypeAliases(path); });
}

Array FactsLogger::getFileModules(const String& path) {
  return logPerf(__func__, [&]() { return m_inner->getFileModules(path); });
}

void FactsLogger::close() {
  return logPerf(__func__, [&]() { m_inner->close(); });
}

Variant FactsLogger::getTypeName(const String& name) {
  return logPerf(__func__, [&]() { return m_inner->getTypeName(name); });
}

Variant FactsLogger::getKind(const String& name) {
  return logPerf(__func__, [&]() { return m_inner->getKind(name); });
}

bool FactsLogger::isTypeAbstract(const String& name) {
  return logPerf(__func__, [&]() { return m_inner->isTypeAbstract(name); });
}

bool FactsLogger::isTypeFinal(const String& name) {
  return logPerf(__func__, [&]() { return m_inner->isTypeFinal(name); });
}

Array FactsLogger::getBaseTypes(const String& name, const Variant& filter) {
  return logPerf(
      __func__, [&]() { return m_inner->getBaseTypes(name, filter); });
}

Array FactsLogger::getDerivedTypes(const String& name, const Variant& filter) {
  return logPerf(
      __func__, [&]() { return m_inner->getDerivedTypes(name, filter); });
}

Array FactsLogger::getTypesWithAttribute(const String& name) {
  return logPerf(
      __func__, [&]() { return m_inner->getTypesWithAttribute(name); });
}

Array FactsLogger::getTypeAliasesWithAttribute(const String& name) {
  return logPerf(
      __func__, [&]() { return m_inner->getTypeAliasesWithAttribute(name); });
}

Array FactsLogger::getMethodsWithAttribute(const String& name) {
  return logPerf(
      __func__, [&]() { return m_inner->getMethodsWithAttribute(name); });
}

Array FactsLogger::getFilesWithAttribute(const String& name) {
  return logPerf(
      __func__, [&]() { return m_inner->getFilesWithAttribute(name); });
}

Array FactsLogger::getFilesWithAttributeAndAnyValue(
    const String& name,
    const folly::dynamic& val) {
  return logPerf(__func__, [&]() {
    return m_inner->getFilesWithAttributeAndAnyValue(name, val);
  });
}

Array FactsLogger::getTypeAttributes(const String& name) {
  return logPerf(__func__, [&]() { return m_inner->getTypeAttributes(name); });
}

Array FactsLogger::getTypeAliasAttributes(const String& name) {
  return logPerf(
      __func__, [&]() { return m_inner->getTypeAliasAttributes(name); });
}

Array FactsLogger::getMethodAttributes(
    const String& type,
    const String& method) {
  return logPerf(
      __func__, [&]() { return m_inner->getMethodAttributes(type, method); });
}

Array FactsLogger::getFileAttributes(const String& path) {
  return logPerf(__func__, [&]() { return m_inner->getFileAttributes(path); });
}

Array FactsLogger::getTypeAttrArgs(const String& type, const String& attr) {
  return logPerf(
      __func__, [&]() { return m_inner->getTypeAttrArgs(type, attr); });
}

Array FactsLogger::getTypeAliasAttrArgs(
    const String& type,
    const String& attr) {
  return logPerf(
      __func__, [&]() { return m_inner->getTypeAliasAttrArgs(type, attr); });
}

Array FactsLogger::getMethodAttrArgs(
    const String& type,
    const String& method,
    const String& attr) {
  return logPerf(__func__, [&]() {
    return m_inner->getMethodAttrArgs(type, method, attr);
  });
}

Array FactsLogger::getFileAttrArgs(const String& file, const String& attr) {
  return logPerf(
      __func__, [&]() { return m_inner->getFileAttrArgs(file, attr); });
}

} // namespace HPHP::Facts
