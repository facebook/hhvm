<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class SpoofChecker {
  const int SINGLE_SCRIPT_CONFUSABLE = 0;
  const int MIXED_SCRIPT_CONFUSABLE = 0;
  const int WHOLE_SCRIPT_CONFUSABLE = 0;
  const int ANY_CASE = 0;
  const int SINGLE_SCRIPT = 0;
  const int INVISIBLE = 0;
  const int CHAR_LIMIT = 0;

  public function __construct();
  public function isSuspicious(string $text, inout $issuesFound);
  public function areConfusable(string $s1, string $s2, inout $issuesFound);
  public function setAllowedLocales(string $localesList);
  public function setChecks(int $checks);
}
