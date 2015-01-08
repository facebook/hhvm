<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
class SpoofChecker {
  const SINGLE_SCRIPT_CONFUSABLE = 0;
  const MIXED_SCRIPT_CONFUSABLE = 0;
  const WHOLE_SCRIPT_CONFUSABLE = 0;
  const ANY_CASE = 0;
  const SINGLE_SCRIPT = 0;
  const INVISIBLE = 0;
  const CHAR_LIMIT = 0;
  public function __construct() { }
  public function isSuspicious($text, &$issuesFound = null) { }
  public function areConfusable($s1, $s2, &$issuesFound = null) { }
  public function setAllowedLocales($localesList) { }
  public function setChecks($checks) { }
}
