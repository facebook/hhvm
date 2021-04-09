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

#include "hphp/runtime/ext/facts/exception.h"

namespace HPHP {
namespace Facts {

FactsExtractionExc::FactsExtractionExc(const std::string& msg)
    : std::runtime_error{msg} {
}

FactsExtractionExc::~FactsExtractionExc() = default;

UpdateExc::UpdateExc(const std::string& msg) : std::runtime_error{msg} {
}

UpdateExc::~UpdateExc() = default;

} // namespace Facts
} // namespace HPHP
