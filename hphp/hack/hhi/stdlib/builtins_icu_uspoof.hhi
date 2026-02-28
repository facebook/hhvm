<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class SpoofChecker {
  const int SINGLE_SCRIPT_CONFUSABLE;
  const int MIXED_SCRIPT_CONFUSABLE;
  const int WHOLE_SCRIPT_CONFUSABLE;
  const int ANY_CASE;
  const int SINGLE_SCRIPT;
  const int INVISIBLE;
  const int CHAR_LIMIT;

  public function __construct();
  public function isSuspicious(
    string $text,
    inout HH\FIXME\MISSING_PARAM_TYPE $issuesFound,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function areConfusable(
    string $s1,
    string $s2,
    inout HH\FIXME\MISSING_PARAM_TYPE $issuesFound,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setAllowedLocales(
    string $localesList,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setChecks(int $checks): HH\FIXME\MISSING_RETURN_TYPE;
}
