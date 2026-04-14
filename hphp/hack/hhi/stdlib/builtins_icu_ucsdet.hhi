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
  public function setText(string $text): void;
  public function setDeclaredEncoding(
    string $text,
  ): void;
  public function detect(): EncodingMatch;
  public function detectAll(): HH\FIXME\MISSING_RETURN_TYPE;
}
class EncodingMatch {
  public function __construct();
  public function isValid(): bool;
  public function getEncoding(): string;
  public function getConfidence(): int;
  public function getLanguage(): string;
  public function getUTF8(): string;
}
