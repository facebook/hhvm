<?hh /* -*- php -*- */
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
  public function setText(string $text): HH\FIXME\MISSING_RETURN_TYPE;
  public function setDeclaredEncoding(
    string $text,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function detect(): HH\FIXME\MISSING_RETURN_TYPE;
  public function detectAll(): HH\FIXME\MISSING_RETURN_TYPE;
}
class EncodingMatch {
  public function __construct();
  public function isValid(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getEncoding(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getConfidence(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getLanguage(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getUTF8(): HH\FIXME\MISSING_RETURN_TYPE;
}
