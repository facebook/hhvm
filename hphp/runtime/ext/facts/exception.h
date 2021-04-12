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

#include <stdexcept>
#include <string>

namespace HPHP {
namespace Facts {

struct FactsExtractionExc : std::runtime_error {
  explicit FactsExtractionExc(const std::string& msg);
  FactsExtractionExc(const FactsExtractionExc&) = default;
  FactsExtractionExc(FactsExtractionExc&&) noexcept = default;
  FactsExtractionExc& operator=(const FactsExtractionExc&) = default;
  FactsExtractionExc& operator=(FactsExtractionExc&&) noexcept = default;
  ~FactsExtractionExc() override;
};

struct UpdateExc : std::runtime_error {
  explicit UpdateExc(const std::string& msg);
  UpdateExc(const UpdateExc&) = default;
  UpdateExc(UpdateExc&&) noexcept = default;
  UpdateExc& operator=(const UpdateExc&) = default;
  UpdateExc& operator=(UpdateExc&&) noexcept = default;
  ~UpdateExc() override;
};

} // namespace Facts
} // namespace HPHP
