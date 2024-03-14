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
#pragma once

#include <memory>
#include "hphp/runtime/base/autoload-map.h"

namespace HPHP::Facts {

struct FactsLogger final : public FactsStore,
                           public std::enable_shared_from_this<FactsLogger> {
  static std::shared_ptr<FactsStore> wrap(
      std::shared_ptr<FactsStore> inner,
      std::string_view impl,
      uint32_t sampleRate,
      uint32_t errorSampleRate);
  FactsLogger(
      std::shared_ptr<FactsStore> inner,
      std::string_view impl,
      uint32_t sampleRate,
      uint32_t errorSampleRate);
  ~FactsLogger() override = default;

  FactsLogger(const FactsLogger&) = delete;
  FactsLogger(FactsLogger&&) noexcept = delete;
  FactsLogger& operator=(const FactsLogger&) = delete;
  FactsLogger& operator=(FactsLogger&&) noexcept = delete;

  void ensureUpdated() override;
  Holder getNativeHolder() noexcept override;

  Optional<FileResult> getTypeOrTypeAliasFile(const String&) override;
  Optional<FileResult> getTypeFile(const String&) override;
  Optional<FileResult> getFunctionFile(const String&) override;
  Optional<FileResult> getConstantFile(const String&) override;
  Optional<FileResult> getTypeAliasFile(const String&) override;
  Optional<FileResult> getModuleFile(const String&) override;

  Optional<std::filesystem::path> getTypeOrTypeAliasFile(
      std::string_view) override;
  Optional<std::filesystem::path> getTypeFile(std::string_view) override;
  Optional<std::filesystem::path> getFunctionFile(std::string_view) override;
  Optional<std::filesystem::path> getConstantFile(std::string_view) override;
  Optional<std::filesystem::path> getTypeAliasFile(std::string_view) override;
  Optional<std::filesystem::path> getModuleFile(std::string_view) override;

  Array getFileTypes(const String&) override;
  Array getFileFunctions(const String&) override;
  Array getFileConstants(const String&) override;
  Array getFileTypeAliases(const String&) override;
  Array getFileModules(const String&) override;

  void close() override;
  Variant getTypeName(const String&) override;
  Variant getKind(const String&) override;
  bool isTypeAbstract(const String&) override;
  bool isTypeFinal(const String&) override;

  Array getBaseTypes(const String&, const Variant&) override;
  Array getDerivedTypes(const String&, const Variant&) override;
  Array getTypesWithAttribute(const String&) override;
  Array getTypeAliasesWithAttribute(const String&) override;
  Array getMethodsWithAttribute(const String&) override;
  Array getFilesWithAttribute(const String&) override;
  Array getFilesAndAttrValsWithAttribute(const String&) override;
  Array getFilesWithAttributeAndAnyValue(const String&, const folly::dynamic&)
      override;
  Array getTypeAttributes(const String&) override;
  Array getTypeAliasAttributes(const String&) override;
  Array getMethodAttributes(const String& type, const String& method) override;
  Array getFileAttributes(const String&) override;
  Array getTypeAttrArgs(const String& type, const String& attr) override;
  Array getTypeAliasAttrArgs(const String& type, const String& attr) override;
  Array getMethodAttrArgs(
      const String& type,
      const String& method,
      const String& attr) override;
  Array getFileAttrArgs(const String& file, const String& attr) override;

 private:
  template <typename F>
  auto logPerf(std::string_view name, std::string_view key, F&& func) const;

 private:
  std::shared_ptr<FactsStore> m_inner;
  std::string m_impl;
  uint32_t m_sampleRate;
  uint32_t m_errorSampleRate;
};

} // namespace HPHP::Facts
