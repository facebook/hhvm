<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class EncodingDetector {
  public function __construct();
  public function setText(string $text);
  public function setDeclaredEncoding(string $text);
  public function detect();
  public function detectAll();
}
class EncodingMatch {
  public function __construct();
  public function isValid();
  public function getEncoding();
  public function getConfidence();
  public function getLanguage();
  public function getUTF8();
}
