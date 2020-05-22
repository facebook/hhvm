/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/hackc-log.h"
#include "hphp/util/struct-log.h"

#include <vector>

namespace HPHP {
namespace HackC {

void logOptions(const folly::dynamic& config_json) {
   // Expect a list of unmerged strings:
   //   [ini_json; set_config1_json; ...; set_configN_json; json_overrides]
   if (!config_json.isArray()) return;

   std::vector<folly::StringPiece> unmerged_configs(config_json.size());
   for (const auto& json : config_json) {
      if (!json.isString()) continue;
      unmerged_configs.push_back(json.stringPiece());
   }
   StructuredLogEntry sle;
   sle.setVec("config_jsons", unmerged_configs);
   StructuredLog::log("hrust_hhbc_opts", sle);
}

} // namespace HackC
} // namespace HPHP
