<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
class NumberFormatter {

  const PATTERN_DECIMAL                    = 0 ;
  const DECIMAL                            = 1 ;
  const CURRENCY                           = 2 ;
  const PERCENT                            = 3 ;
  const SCIENTIFIC                         = 4 ;
  const SPELLOUT                           = 5 ;
  const ORDINAL                            = 6 ;
  const DURATION                           = 7 ;
  const PATTERN_RULEBASED                  = 8 ;
  const IGNORE                             = 9 ;
  const DEFAULT_STYLE                      = 10;
  const ROUND_CEILING                      = 11;
  const ROUND_FLOOR                        = 12;
  const ROUND_DOWN                         = 13;
  const ROUND_UP                           = 14;
  const ROUND_HALFEVEN                     = 15;
  const ROUND_HALFDOWN                     = 16;
  const ROUND_HALFUP                       = 17;
  const PAD_BEFORE_PREFIX                  = 18;
  const PAD_AFTER_PREFIX                   = 19;
  const PAD_BEFORE_SUFFIX                  = 20;
  const PAD_AFTER_SUFFIX                   = 21;
  const PARSE_INT_ONLY                     = 22;
  const GROUPING_USED                      = 23;
  const DECIMAL_ALWAYS_SHOWN               = 24;
  const MAX_INTEGER_DIGITS                 = 25;
  const MIN_INTEGER_DIGITS                 = 26;
  const INTEGER_DIGITS                     = 27;
  const MAX_FRACTION_DIGITS                = 28;
  const MIN_FRACTION_DIGITS                = 29;
  const FRACTION_DIGITS                    = 30;
  const MULTIPLIER                         = 31;
  const GROUPING_SIZE                      = 32;
  const ROUNDING_MODE                      = 33;
  const ROUNDING_INCREMENT                 = 34;
  const FORMAT_WIDTH                       = 35;
  const PADDING_POSITION                   = 36;
  const SECONDARY_GROUPING_SIZE            = 37;
  const SIGNIFICANT_DIGITS_USED            = 38;
  const MIN_SIGNIFICANT_DIGITS             = 39;
  const MAX_SIGNIFICANT_DIGITS             = 40;
  const LENIENT_PARSE                      = 41;
  const POSITIVE_PREFIX                    = 42;
  const POSITIVE_SUFFIX                    = 43;
  const NEGATIVE_PREFIX                    = 44;
  const NEGATIVE_SUFFIX                    = 45;
  const PADDING_CHARACTER                  = 46;
  const CURRENCY_CODE                      = 47;
  const DEFAULT_RULESET                    = 48;
  const PUBLIC_RULESETS                    = 49;
  const DECIMAL_SEPARATOR_SYMBOL           = 50;
  const GROUPING_SEPARATOR_SYMBOL          = 51;
  const PATTERN_SEPARATOR_SYMBOL           = 52;
  const PERCENT_SYMBOL                     = 53;
  const ZERO_DIGIT_SYMBOL                  = 54;
  const DIGIT_SYMBOL                       = 55;
  const MINUS_SIGN_SYMBOL                  = 56;
  const PLUS_SIGN_SYMBOL                   = 57;
  const CURRENCY_SYMBOL                    = 58;
  const INTL_CURRENCY_SYMBOL               = 59;
  const MONETARY_SEPARATOR_SYMBOL          = 60;
  const EXPONENTIAL_SYMBOL                 = 61;
  const PERMILL_SYMBOL                     = 62;
  const PAD_ESCAPE_SYMBOL                  = 63;
  const INFINITY_SYMBOL                    = 64;
  const NAN_SYMBOL                         = 65;
  const SIGNIFICANT_DIGIT_SYMBOL           = 66;
  const MONETARY_GROUPING_SEPARATOR_SYMBOL = 67;
  const TYPE_DEFAULT                       = 68;
  const TYPE_INT32                         = 69;
  const TYPE_INT64                         = 70;
  const TYPE_DOUBLE                        = 71;
  const TYPE_CURRENCY                      = 72;

  <<__Rx>>
  public function __construct(string $locale, int $style, string $pattern = "#,##0.###");
  <<__Rx, __MaybeMutable>>
  public function formatCurrency(float $value, string $currency) : string;
  <<__Rx, __MaybeMutable>>
 public function format(mixed $value, int $type = NumberFormatter::TYPE_DEFAULT) : string;
  <<__Rx, __MaybeMutable>>
  public function getAttribute(int $attr) : int;
  <<__Rx, __MaybeMutable>>
  public function getErrorCode() : int;
  <<__Rx, __MaybeMutable>>
  public function getErrorMessage() : string;
  <<__Rx, __MaybeMutable>>
  public function getLocale(int $type = Locale::ACTUAL_LOCALE) : string;
  <<__Rx, __MaybeMutable>>
  public function getPattern() : string;
  <<__Rx, __MaybeMutable>>
  public function getSymbol(int $attr) : string;
  <<__Rx, __MaybeMutable>>
  public function getTextAttribute(int $attr) : string;
  public function parseCurrency(string $value, inout string $currency, inout int $position) : float;
  public function parse(string $value, int $type = NumberFormatter::TYPE_DOUBLE) : mixed;
  public function parseWithPosition(string $value, int $type, inout int $position) : mixed;
  <<__Rx, __Mutable>>
  public function setAttribute(int $attr, int $value) : bool;
  <<__Rx, __Mutable>>
  public function setPattern(string $pattern) : bool;
  <<__Rx, __Mutable>>
  public function setSymbol(int $attr, string $value) : bool;
  <<__Rx, __Mutable>>
  public function setTextAttribute(int $attr, string $value) : bool;
}
