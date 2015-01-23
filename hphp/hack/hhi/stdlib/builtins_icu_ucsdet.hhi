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
class EncodingDetector {
  public function __construct() { }
  public function setText($text) { }
  public function setDeclaredEncoding($text) { }
  public function detect() { }
  public function detectAll() { }
}
class EncodingMatch {
  public function __construct() { }
  public function isValid() { }
  public function getEncoding() { }
  public function getConfidence() { }
  public function getLanguage() { }
  public function getUTF8() { }
}
