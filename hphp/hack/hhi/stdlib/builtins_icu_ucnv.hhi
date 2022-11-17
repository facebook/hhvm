<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class UConverter {
  const int REASON_UNASSIGNED;
  const int REASON_ILLEGAL;
  const int REASON_IRREGULAR;
  const int REASON_RESET;
  const int REASON_CLOSE;
  const int REASON_CLONE;
  const int UNSUPPORTED_CONVERTER;
  const int SBCS;
  const int DBCS;
  const int MBCS;
  const int LATIN_1;
  const int UTF8;
  const int UTF16_BigEndian;
  const int UTF16_LittleEndian;
  const int UTF32_BigEndian;
  const int UTF32_LittleEndian;
  const int EBCDIC_STATEFUL;
  const int ISO_2022;
  const int LMBCS_1;
  const int LMBCS_2;
  const int LMBCS_3;
  const int LMBCS_4;
  const int LMBCS_5;
  const int LMBCS_6;
  const int LMBCS_8;
  const int LMBCS_11;
  const int LMBCS_16;
  const int LMBCS_17;
  const int LMBCS_18;
  const int LMBCS_19;
  const int LMBCS_LAST;
  const int HZ;
  const int SCSU;
  const int ISCII;
  const int US_ASCII;
  const int UTF7;
  const int BOCU1;
  const int UTF16;
  const int UTF32;
  const int CESU8;
  const int IMAP_MAILBOX;

  public function __construct(
    string $toEncoding = "utf-8",
    string $fromEncoding = "utf-8",
  );
  public function getSourceEncoding(): HH\FIXME\MISSING_RETURN_TYPE;
  public function setSourceEncoding(
    string $encoding,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDestinationEncoding(): HH\FIXME\MISSING_RETURN_TYPE;
  public function setDestinationEncoding(
    string $encoding,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getSourceType(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getDestinationType(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getSubstChars(): HH\FIXME\MISSING_RETURN_TYPE;
  public function setSubstChars(string $chars): HH\FIXME\MISSING_RETURN_TYPE;
  public function fromUCallback(
    int $reason,
    $source,
    int $codepoint,
    inout int $error,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function toUCallback(
    int $reason,
    $source,
    $codeunits,
    inout int $error,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function convert(
    string $str,
    bool $reverse = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  static public function transcode(
    string $str,
    string $toEncoding,
    string $fromEncoding,
    $options = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getErrorMessage(): HH\FIXME\MISSING_RETURN_TYPE;
  static public function reasonText(int $reason): HH\FIXME\MISSING_RETURN_TYPE;
  static public function getAvailable(): HH\FIXME\MISSING_RETURN_TYPE;
  static public function getAliases(
    string $encoding,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  static public function getStandards(): HH\FIXME\MISSING_RETURN_TYPE;
  static public function getStandardName(
    string $name,
    string $standard,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  static public function getMimeName(
    string $name,
  ): HH\FIXME\MISSING_RETURN_TYPE;
}
