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

  const int PATTERN_DECIMAL                    = 0 ;
  const int DECIMAL                            = 1 ;
  const int CURRENCY                           = 2 ;
  const int PERCENT                            = 3 ;
  const int SCIENTIFIC                         = 4 ;
  const int SPELLOUT                           = 5 ;
  const int ORDINAL                            = 6 ;
  const int DURATION                           = 7 ;
  const int PATTERN_RULEBASED                  = 8 ;
  const int IGNORE                             = 9 ;
  const int DEFAULT_STYLE                      = 10;
  const int ROUND_CEILING                      = 11;
  const int ROUND_FLOOR                        = 12;
  const int ROUND_DOWN                         = 13;
  const int ROUND_UP                           = 14;
  const int ROUND_HALFEVEN                     = 15;
  const int ROUND_HALFDOWN                     = 16;
  const int ROUND_HALFUP                       = 17;
  const int PAD_BEFORE_PREFIX                  = 18;
  const int PAD_AFTER_PREFIX                   = 19;
  const int PAD_BEFORE_SUFFIX                  = 20;
  const int PAD_AFTER_SUFFIX                   = 21;
  const int PARSE_INT_ONLY                     = 22;
  const int GROUPING_USED                      = 23;
  const int DECIMAL_ALWAYS_SHOWN               = 24;
  const int MAX_INTEGER_DIGITS                 = 25;
  const int MIN_INTEGER_DIGITS                 = 26;
  const int INTEGER_DIGITS                     = 27;
  const int MAX_FRACTION_DIGITS                = 28;
  const int MIN_FRACTION_DIGITS                = 29;
  const int FRACTION_DIGITS                    = 30;
  const int MULTIPLIER                         = 31;
  const int GROUPING_SIZE                      = 32;
  const int ROUNDING_MODE                      = 33;
  const int ROUNDING_INCREMENT                 = 34;
  const int FORMAT_WIDTH                       = 35;
  const int PADDING_POSITION                   = 36;
  const int SECONDARY_GROUPING_SIZE            = 37;
  const int SIGNIFICANT_DIGITS_USED            = 38;
  const int MIN_SIGNIFICANT_DIGITS             = 39;
  const int MAX_SIGNIFICANT_DIGITS             = 40;
  const int LENIENT_PARSE                      = 41;
  const int POSITIVE_PREFIX                    = 42;
  const int POSITIVE_SUFFIX                    = 43;
  const int NEGATIVE_PREFIX                    = 44;
  const int NEGATIVE_SUFFIX                    = 45;
  const int PADDING_CHARACTER                  = 46;
  const int CURRENCY_CODE                      = 47;
  const int DEFAULT_RULESET                    = 48;
  const int PUBLIC_RULESETS                    = 49;
  const int DECIMAL_SEPARATOR_SYMBOL           = 50;
  const int GROUPING_SEPARATOR_SYMBOL          = 51;
  const int PATTERN_SEPARATOR_SYMBOL           = 52;
  const int PERCENT_SYMBOL                     = 53;
  const int ZERO_DIGIT_SYMBOL                  = 54;
  const int DIGIT_SYMBOL                       = 55;
  const int MINUS_SIGN_SYMBOL                  = 56;
  const int PLUS_SIGN_SYMBOL                   = 57;
  const int CURRENCY_SYMBOL                    = 58;
  const int INTL_CURRENCY_SYMBOL               = 59;
  const int MONETARY_SEPARATOR_SYMBOL          = 60;
  const int EXPONENTIAL_SYMBOL                 = 61;
  const int PERMILL_SYMBOL                     = 62;
  const int PAD_ESCAPE_SYMBOL                  = 63;
  const int INFINITY_SYMBOL                    = 64;
  const int NAN_SYMBOL                         = 65;
  const int SIGNIFICANT_DIGIT_SYMBOL           = 66;
  const int MONETARY_GROUPING_SEPARATOR_SYMBOL = 67;
  const int TYPE_DEFAULT                       = 68;
  const int TYPE_INT32                         = 69;
  const int TYPE_INT64                         = 70;
  const int TYPE_DOUBLE                        = 71;
  const int TYPE_CURRENCY                      = 72;

  public function __construct(string $locale, int $style, string $pattern = "#,##0.###");
  public function formatCurrency(float $value, string $currency)[]: string;
 public function format(mixed $value, int $type = NumberFormatter::TYPE_DEFAULT)[]: string;
  public function getAttribute(int $attr)[]: int;
  public function getErrorCode()[]: int;
  public function getErrorMessage()[]: string;
  public function getLocale(int $type = Locale::ACTUAL_LOCALE)[]: string;
  public function getPattern()[]: string;
  public function getSymbol(int $attr)[]: string;
  public function getTextAttribute(int $attr)[]: string;
  public function parseCurrency(string $value, inout string $currency, inout int $position) : float;
  public function parse(string $value, int $type = NumberFormatter::TYPE_DOUBLE) : mixed;
  public function parseWithPosition(string $value, int $type, inout int $position) : mixed;
  public function setAttribute(int $attr, int $value)[]: bool;
  public function setPattern(string $pattern)[]: bool;
  public function setSymbol(int $attr, string $value)[]: bool;
  public function setTextAttribute(int $attr, string $value)[]: bool;
}
