<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class UConverter {
  const int REASON_UNASSIGNED = 0;
  const int REASON_ILLEGAL = 0;
  const int REASON_IRREGULAR = 0;
  const int REASON_RESET = 0;
  const int REASON_CLOSE = 0;
  const int REASON_CLONE = 0;
  const int UNSUPPORTED_CONVERTER = 0;
  const int SBCS = 0;
  const int DBCS = 0;
  const int MBCS = 0;
  const int LATIN_1 = 0;
  const int UTF8 = 0;
  const int UTF16_BigEndian = 0;
  const int UTF16_LittleEndian = 0;
  const int UTF32_BigEndian = 0;
  const int UTF32_LittleEndian = 0;
  const int EBCDIC_STATEFUL = 0;
  const int ISO_2022 = 0;
  const int LMBCS_1 = 0;
  const int LMBCS_2 = 0;
  const int LMBCS_3 = 0;
  const int LMBCS_4 = 0;
  const int LMBCS_5 = 0;
  const int LMBCS_6 = 0;
  const int LMBCS_8 = 0;
  const int LMBCS_11 = 0;
  const int LMBCS_16 = 0;
  const int LMBCS_17 = 0;
  const int LMBCS_18 = 0;
  const int LMBCS_19 = 0;
  const int LMBCS_LAST = 0;
  const int HZ = 0;
  const int SCSU = 0;
  const int ISCII = 0;
  const int US_ASCII = 0;
  const int UTF7 = 0;
  const int BOCU1 = 0;
  const int UTF16 = 0;
  const int UTF32 = 0;
  const int CESU8 = 0;
  const int IMAP_MAILBOX = 0;

  public function __construct(string $toEncoding = "utf-8", string $fromEncoding = "utf-8");
  public function getSourceEncoding();
  public function setSourceEncoding(string $encoding);
  public function getDestinationEncoding();
  public function setDestinationEncoding(string $encoding);
  public function getSourceType();
  public function getDestinationType();
  public function getSubstChars();
  public function setSubstChars(string $chars);
  public function fromUCallback(int $reason, $source, int $codepoint, inout int $error);
  public function toUCallback(int $reason, $source, $codeunits, inout int $error);
  public function convert(string $str, bool $reverse = false);
  static public function transcode(string $str, string $toEncoding, string $fromEncoding, $options = null);
  public function getErrorCode();
  public function getErrorMessage();
  static public function reasonText(int $reason);
  static public function getAvailable();
  static public function getAliases(string $encoding);
  static public function getStandards();
  static public function getStandardName(string $name, string $standard);
  static public function getMimeName(string $name);
}
