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

  void validate(const std::set<std::string>& types_to_ignore) override;
  Holder getNativeHolder() noexcept override;

  Optional<FileResult> getTypeOrTypeAliasFile(const OptString&) override;
  Optional<FileResult> getTypeOrTypeAliasFileRelative(
      const OptString&) override;
  Optional<FileResult> getTypeFile(const OptString&) override;
  Optional<FileResult> getTypeFileRelative(const OptString&) override;
  Optional<FileResult> getFunctionFile(const OptString&) override;
  Optional<FileResult> getFunctionFileRelative(const OptString&) override;
  Optional<FileResult> getConstantFile(const OptString&) override;
  Optional<FileResult> getConstantFileRelative(const OptString&) override;
  Optional<FileResult> getTypeAliasFile(const OptString&) override;
  Optional<FileResult> getTypeAliasFileRelative(const OptString&) override;
  Optional<FileResult> getModuleFile(const OptString&) override;
  Optional<FileResult> getModuleFileRelative(const OptString&) override;

  Optional<std::filesystem::path> getTypeOrTypeAliasFile(
      std::string_view) override;
  Optional<std::filesystem::path> getTypeOrTypeAliasFileRelative(
      std::string_view) override;
  Optional<std::filesystem::path> getTypeFile(std::string_view) override;
  Optional<std::filesystem::path> getTypeFileRelative(
      std::string_view) override;
  Optional<std::filesystem::path> getFunctionFile(std::string_view) override;
  Optional<std::filesystem::path> getFunctionFileRelative(
      std::string_view) override;
  Optional<std::filesystem::path> getConstantFile(std::string_view) override;
  Optional<std::filesystem::path> getConstantFileRelative(
      std::string_view) override;
  Optional<std::filesystem::path> getTypeAliasFile(std::string_view) override;
  Optional<std::filesystem::path> getTypeAliasFileRelative(
      std::string_view) override;
  Optional<std::filesystem::path> getModuleFile(std::string_view) override;
  Optional<std::filesystem::path> getModuleFileRelative(
      std::string_view) override;

  Optional<std::string> getSha1(const OptString& path) override;

  Array getFileTypes(const OptString&) override;
  Array getFileFunctions(const OptString&) override;
  Array getFileConstants(const OptString&) override;
  Array getFileTypeAliases(const OptString&) override;
  Array getFileModules(const OptString&) override;
  Array getAllModules() override;

  Optional<OptString> getFileModuleMembership(const OptString& path) override;
  Optional<OptString> getFilePackageMembership(const OptString& path) override;

  void close() override;
  OptString getTypeName(const OptString&) override;
  Variant getKind(const OptString&) override;
  bool isTypeAbstract(const OptString&) override;
  bool isTypeFinal(const OptString&) override;

  Array getBaseTypes(const OptString&, const Variant&) override;
  Array getDerivedTypes(const OptString&, const Variant&) override;
  Array getTransitiveDerivedTypes(
      const OptString&,
      const Variant&,
      bool includeInterfaceRequireExtends) override;
  Array getTypesWithAttribute(const OptString&) override;
  Array getTypeAliasesWithAttribute(const OptString&) override;
  Array getMethodsWithAttribute(const OptString&) override;
  Array getTypeMethodAttributes(const OptString&) override;
  Array getFilesWithAttribute(const OptString&) override;
  Array getFilesAndAttrValsWithAttribute(const OptString&) override;
  Array getFilesWithAttributeAndAnyValue(
      const OptString&,
      const folly::dynamic&) override;
  Array getTypeAttributes(const OptString&) override;
  Array getTypeAliasAttributes(const OptString&) override;
  Array getMethodAttributes(const OptString& type, const OptString& method)
      override;
  Array getFileAttributes(const OptString&) override;
  Array getTypeAttrArgs(const OptString& type, const OptString& attr) override;
  Array getTypeAliasAttrArgs(const OptString& type, const OptString& attr)
      override;
  Array getMethodAttrArgs(
      const OptString& type,
      const OptString& method,
      const OptString& attr) override;
  Array getFileAttrArgs(const OptString& file, const OptString& attr) override;

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
