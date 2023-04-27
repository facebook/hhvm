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

  const int PATTERN_DECIMAL;
  const int DECIMAL;
  const int CURRENCY;
  const int PERCENT;
  const int SCIENTIFIC;
  const int SPELLOUT;
  const int ORDINAL;
  const int DURATION;
  const int PATTERN_RULEBASED;
  const int IGNORE;
  const int DEFAULT_STYLE;
  const int ROUND_CEILING;
  const int ROUND_FLOOR;
  const int ROUND_DOWN;
  const int ROUND_UP;
  const int ROUND_HALFEVEN;
  const int ROUND_HALFDOWN;
  const int ROUND_HALFUP;
  const int PAD_BEFORE_PREFIX;
  const int PAD_AFTER_PREFIX;
  const int PAD_BEFORE_SUFFIX;
  const int PAD_AFTER_SUFFIX;
  const int PARSE_INT_ONLY;
  const int GROUPING_USED;
  const int DECIMAL_ALWAYS_SHOWN;
  const int MAX_INTEGER_DIGITS;
  const int MIN_INTEGER_DIGITS;
  const int INTEGER_DIGITS;
  const int MAX_FRACTION_DIGITS;
  const int MIN_FRACTION_DIGITS;
  const int FRACTION_DIGITS;
  const int MULTIPLIER;
  const int GROUPING_SIZE;
  const int ROUNDING_MODE;
  const int ROUNDING_INCREMENT;
  const int FORMAT_WIDTH;
  const int PADDING_POSITION;
  const int SECONDARY_GROUPING_SIZE;
  const int SIGNIFICANT_DIGITS_USED;
  const int MIN_SIGNIFICANT_DIGITS;
  const int MAX_SIGNIFICANT_DIGITS;
  const int LENIENT_PARSE;
  const int POSITIVE_PREFIX;
  const int POSITIVE_SUFFIX;
  const int NEGATIVE_PREFIX;
  const int NEGATIVE_SUFFIX;
  const int PADDING_CHARACTER;
  const int CURRENCY_CODE;
  const int DEFAULT_RULESET;
  const int PUBLIC_RULESETS;
  const int DECIMAL_SEPARATOR_SYMBOL;
  const int GROUPING_SEPARATOR_SYMBOL;
  const int PATTERN_SEPARATOR_SYMBOL;
  const int PERCENT_SYMBOL;
  const int ZERO_DIGIT_SYMBOL;
  const int DIGIT_SYMBOL;
  const int MINUS_SIGN_SYMBOL;
  const int PLUS_SIGN_SYMBOL;
  const int CURRENCY_SYMBOL;
  const int INTL_CURRENCY_SYMBOL;
  const int MONETARY_SEPARATOR_SYMBOL;
  const int EXPONENTIAL_SYMBOL;
  const int PERMILL_SYMBOL;
  const int PAD_ESCAPE_SYMBOL;
  const int INFINITY_SYMBOL;
  const int NAN_SYMBOL;
  const int SIGNIFICANT_DIGIT_SYMBOL;
  const int MONETARY_GROUPING_SEPARATOR_SYMBOL;
  const int TYPE_DEFAULT;
  const int TYPE_INT32;
  const int TYPE_INT64;
  const int TYPE_DOUBLE;
  const int TYPE_CURRENCY;

  public function __construct(
    string $locale,
    int $style,
    string $pattern = "#,##0.###",
  );
  public function formatCurrency(float $value, string $currency)[]: string;
  public function format(
    mixed $value,
    int $type = NumberFormatter::TYPE_DEFAULT,
  )[]: string;
  public function getAttribute(int $attr)[]: int;
  public function getErrorCode()[]: int;
  public function getErrorMessage()[]: string;
  public function getLocale(int $type = Locale::ACTUAL_LOCALE)[]: string;
  public function getPattern()[]: string;
  public function getSymbol(int $attr)[]: string;
  public function getTextAttribute(int $attr)[]: string;
  public function parseCurrency(
    string $value,
    inout string $currency,
    inout int $position,
  ): float;
  public function parse(
    string $value,
    int $type = NumberFormatter::TYPE_DOUBLE,
  ): mixed;
  public function parseWithPosition(
    string $value,
    int $type,
    inout int $position,
  ): mixed;
  public function setAttribute(int $attr, int $value)[]: bool;
  public function setPattern(string $pattern)[]: bool;
  public function setSymbol(int $attr, string $value)[]: bool;
  public function setTextAttribute(int $attr, string $value)[]: bool;
}
