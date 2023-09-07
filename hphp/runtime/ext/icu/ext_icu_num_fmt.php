<?hh // partial

/**
 * Programs store and operate on numbers using a locale-independent binary
 * representation. When displaying or printing a number it is converted to a
 * locale-specific string. For example, the number 12345.67 is "12,345.67" in
 * the US, "12 345,67" in France and "12.345,67" in Germany.   By invoking the
 * methods provided by the NumberFormatter class, you can format numbers,
 * currencies, and percentages according to the specified or default locale.
 * NumberFormatter is locale-sensitive so you need to create a new
 * NumberFormatter for each locale. NumberFormatter methods format
 * primitive-type numbers, such as double and output the number as a
 * locale-specific string.   For currencies you can use currency format type
 * to create a formatter that returns a string with the formatted number and
 * the appropriate currency sign. Of course, the NumberFormatter class is
 * unaware of exchange rates so, the number output is the same regardless of
 * the specified currency. This means that the same number has different
 * monetary values depending on the currency locale. If the number is
 * 9988776.65 the results will be:  9 988 776,65 € in France 9.988.776,65
 * € in Germany $9,988,776.65 in the United States    In order to format
 * percentages, create a locale-specific formatter with percentage format
 * type. With this formatter, a decimal fraction such as 0.75 is displayed as
 * 75%.   For more complex formatting, like spelled-out numbers, the
 * rule-based number formatters are used.
 */
<<__NativeData>>
class NumberFormatter {
  /**
   * Create a number formatter
   *
   * @param string $locale - Locale in which the number would be
   *   formatted (locale name, e.g. en_CA).
   * @param int $style - Style of the formatting, one of the  format
   *   style constants. If NumberFormatter::PATTERN_DECIMAL or
   *   NumberFormatter::PATTERN_RULEBASED is passed then the number format
   *   is opened using the given pattern, which must conform to the syntax
   *   described in ICU DecimalFormat documentation or ICU
   *   RuleBasedNumberFormat documentation, respectively.
   * @param string $pattern - Pattern string if the chosen style requires
   *   a pattern.
   *
   * @return NumberFormatter - Returns NumberFormatter object or FALSE on
   *   error.
   */
  <<__Native>>
  public function __construct(string $locale, int $style,
                              string $pattern = ""): void;

  public static function create($locale,
                                $style,
                                $pattern = "") {
    try {
      return new NumberFormatter($locale, $style, $pattern);
    } catch (Exception $e) {
      return false;
    }
  }

  /**
   * Format a currency value
   *
   * @param float $value - The numeric currency value.
   * @param string $currency - The 3-letter ISO 4217 currency code
   *   indicating the currency to use.
   *
   * @return string - String representing the formatted currency value.
   */
  <<__Native>>
  public function formatCurrency(float $value,
                                 string $currency)[]: string;

  /**
   * Format a number
   *
   * @param number $value - The value to format. Can be integer or float,
   *   other values will be converted to a numeric value.
   * @param int $type - The  formatting type to use.
   *
   * @return string - Returns the string containing formatted value, or
   *   FALSE on error.
   */
  <<__Native>>
  public function format(mixed $value,
                         int $type = NumberFormatter::TYPE_DEFAULT)[]: mixed;

  /**
   * Get an attribute
   *
   * @param int $attr - Attribute specifier - one of the  numeric
   *   attribute constants.
   *
   * @return mixed - Return attribute value on success, or FALSE on error.
   */
  <<__Native>>
  public function getAttribute(int $attr)[]: mixed;

  /**
   * Get formatter's last error code.
   *
   * @return int - Returns error code from last formatter call.
   */
  <<__Native>>
  public function getErrorCode()[]: int;

  /**
   * Get formatter's last error message.
   *
   * @return string - Returns error message from last formatter call.
   */
  <<__Native>>
  public function getErrorMessage()[]: string;

  /**
   * Get formatter locale
   *
   * @param int $type - You can choose between valid and actual locale (
   *   Locale::VALID_LOCALE, Locale::ACTUAL_LOCALE, respectively). The
   *   default is the actual locale.
   *
   * @return string - The locale name used to create the formatter.
   */
  <<__Native>>
  public function getLocale(int $type = ULOC_ACTUAL_LOCALE)[]: string;

  /**
   * Get formatter pattern
   *
   * @return string - Pattern string that is used by the formatter, or
   *   FALSE if an error happens.
   */
  <<__Native>>
  public function getPattern()[]: string;

  /**
   * Get a symbol value
   *
   * @param int $attr - Symbol specifier, one of the  format symbol
   *   constants.
   *
   * @return string - The symbol string or FALSE on error.
   */
  <<__Native>>
  public function getSymbol(int $attr)[]: string;

  /**
   * Get a text attribute
   *
   * @param int $attr - Attribute specifier - one of the  text attribute
   *   constants.
   *
   * @return string - Return attribute value on success, or FALSE on
   *   error.
   */
  <<__Native>>
  public function getTextAttribute(int $attr)[]: string;

  /**
   * Parse a currency number
   *
   * @param string $value -
   * @param string $currency - Parameter to receive the currency name
   *   (3-letter ISO 4217 currency code).
   * @param int $position - Offset in the string at which to begin
   *   parsing. On return, this value will hold the offset at which parsing
   *   ended.
   *
   * @return float - The parsed numeric value or FALSE on error.
   */
  <<__Native>>
  public function parseCurrency(string $value,
                                <<__OutOnly("KindOfString")>>
                                inout mixed $currency,
                                inout mixed $position): mixed;

  /**
   * Parse a number
   *
   * @param string $value -
   * @param int $type - The  formatting type to use. By default,
   *   NumberFormatter::TYPE_DOUBLE is used.
   * @param int $position - Offset in the string at which to begin
   *   parsing. On return, this value will hold the offset at which parsing
   *   ended.
   *
   * @return mixed - The value of the parsed number or FALSE on error.
   */
  public function parse(string $value,
                        int $type = NumberFormatter::TYPE_DOUBLE): mixed {
    $position = null;
    $result = $this->parseWithPosition($value, $type, inout $position);
    return $result;
  }

  <<__Native>>
  public function parseWithPosition(string $value,
                                    int $type,
                                    inout mixed $position): mixed;

  /**
   * Set an attribute
   *
   * @param int $attr - Attribute specifier - one of the  numeric
   *   attribute constants.
   * @param mixed $value - The attribute value.
   *
   * @return bool -
   */
  <<__Native>>
  public function setAttribute(int $attr,
                               mixed $value)[]: bool;

  /**
   * Set formatter pattern
   *
   * @param string $pattern - Pattern in syntax described in ICU
   *   DecimalFormat documentation.
   *
   * @return bool -
   */
  <<__Native>>
  public function setPattern(string $pattern)[]: bool;

  /**
   * Set a symbol value
   *
   * @param int $attr - Symbol specifier, one of the  format symbol
   *   constants.
   * @param string $value - Text for the symbol.
   *
   * @return bool -
   */
  <<__Native>>
  public function setSymbol(int $attr,
                            string $value)[]: bool;

  /**
   * Set a text attribute
   *
   * @param int $attr - Attribute specifier - one of the text attribute
   *   constants.
   * @param string $value - Text for the attribute value.
   *
   * @return bool -
   */
  <<__Native>>
  public function setTextAttribute(int $attr,
                                   string $value)[]: bool;

}

/**
 * Create a number formatter
 *
 * @param string $locale - Locale in which the number would be formatted
 *   (locale name, e.g. en_CA).
 * @param int $style - Style of the formatting, one of the  format style
 *   constants. If NumberFormatter::PATTERN_DECIMAL or
 *   NumberFormatter::PATTERN_RULEBASED is passed then the number format is
 *   opened using the given pattern, which must conform to the syntax
 *   described in ICU DecimalFormat documentation or ICU
 *   RuleBasedNumberFormat documentation, respectively.
 * @param string $pattern - Pattern string if the chosen style requires a
 *   pattern.
 *
 * @return NumberFormatter - Returns NumberFormatter object or FALSE on
 *   error.
 */
function numfmt_create($locale, $style, $pattern = ""): NumberFormatter {
  return NumberFormatter::create($locale, $style, $pattern);
}

/**
 * Format a currency value
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param float $value - The numeric currency value.
 * @param string $currency - The 3-letter ISO 4217 currency code
 *   indicating the currency to use.
 *
 * @return string - String representing the formatted currency value.
 */
function numfmt_format_currency(NumberFormatter $fmt,
                                $value, $currency): mixed {
  return $fmt->formatCurrency($value, $currency);
}

/**
 * Format a number
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param number $value - The value to format. Can be integer or float,
 *   other values will be converted to a numeric value.
 * @param int $type - The  formatting type to use.
 *
 * @return string - Returns the string containing formatted value, or
 *   FALSE on error.
 */
function numfmt_format(NumberFormatter $fmt,
                       $value, $type = NumberFormatter::TYPE_DEFAULT) {
  return $fmt->format($value, $type);
}

/**
 * Get an attribute
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param int $attr - Attribute specifier - one of the  numeric attribute
 *   constants.
 *
 * @return int - Return attribute value on success, or FALSE on error.
 */
function numfmt_get_attribute(NumberFormatter $fmt, $attr): mixed {
  return $fmt->getAttribute($attr);
}

/**
 * Get formatter's last error code.
 *
 * @param numberformatter $fmt - NumberFormatter object.
 *
 * @return int - Returns error code from last formatter call.
 */
function numfmt_get_error_code(NumberFormatter $fmt): int {
  return $fmt->getErrorCode();
}

/**
 * Get formatter's last error message.
 *
 * @param numberformatter $fmt - NumberFormatter object.
 *
 * @return string - Returns error message from last formatter call.
 */
function numfmt_get_error_message(NumberFormatter $fmt): string {
  return $fmt->getErrorMessage();
}

/**
 * Get formatter locale
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param int $type - You can choose between valid and actual locale (
 *   Locale::VALID_LOCALE, Locale::ACTUAL_LOCALE, respectively). The
 *   default is the actual locale.
 *
 * @return string - The locale name used to create the formatter.
 */
function numfmt_get_locale(NumberFormatter $fmt,
                           $type = ULOC_ACTUAL_LOCALE): string {
  return $fmt->getLocale($type);
}

/**
 * Get formatter pattern
 *
 * @param numberformatter $fmt - NumberFormatter object.
 *
 * @return string - Pattern string that is used by the formatter, or
 *   FALSE if an error happens.
 */
function numfmt_get_pattern(NumberFormatter $fmt) {
  return $fmt->getPattern();
}

/**
 * Get a symbol value
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param int $attr - Symbol specifier, one of the  format symbol
 *   constants.
 *
 * @return string - The symbol string or FALSE on error.
 */
function numfmt_get_symbol(NumberFormatter $fmt, $attr) {
  return $fmt->getSymbol($attr);
}

/**
 * Get a text attribute
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param int $attr - Attribute specifier - one of the  text attribute
 *   constants.
 *
 * @return string - Return attribute value on success, or FALSE on
 *   error.
 */
function numfmt_get_text_attribute(NumberFormatter $fmt, $attr) {
  return $fmt->getTextAttribute($attr);
}

/**
 * Parse a currency number
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param string $value -
 * @param string $currency - Parameter to receive the currency name
 *   (3-letter ISO 4217 currency code).
 * @param int $position - Offset in the string at which to begin parsing.
 *   On return, this value will hold the offset at which parsing ended.
 *
 * @return float - The parsed numeric value or FALSE on error.
 */
function numfmt_parse_currency(NumberFormatter $fmt,
                               $value,
                               inout $currency,
                               inout $position): mixed {
  return $fmt->parseCurrency($value, inout $currency, inout $position);
}

/**
 * Parse a number
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param string $value -
 * @param int $type - The  formatting type to use. By default,
 *   NumberFormatter::TYPE_DOUBLE is used.
 * @param int $position - Offset in the string at which to begin parsing.
 *   On return, this value will hold the offset at which parsing ended.
 *
 * @return mixed - The value of the parsed number or FALSE on error.
 */
function numfmt_parse(NumberFormatter $fmt,
                      $value,
                      $type,
                      inout $position): mixed {
  return $fmt->parseWithPosition($value, $type, inout $position);
}

/**
 * Set an attribute
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param int $attr - Attribute specifier - one of the  numeric attribute
 *   constants.
 * @param mixed $value - The attribute value.
 *
 * @return bool -
 */
function numfmt_set_attribute(NumberFormatter $fmt,
                              $attr,
                              $value): bool {
  return $fmt->setAttribute($attr, $value);
}

/**
 * Set formatter pattern
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param string $pattern - Pattern in syntax described in ICU
 *   DecimalFormat documentation.
 *
 * @return bool -
 */
function numfmt_set_pattern(NumberFormatter $fmt,
                            $pattern): bool {
  return $fmt->setPattern($pattern);
}

/**
 * Set a symbol value
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param int $attr - Symbol specifier, one of the  format symbol
 *   constants.
 * @param string $value - Text for the symbol.
 *
 * @return bool -
 */
function numfmt_set_symbol(NumberFormatter $fmt,
                           $attr,
                           $value): bool {
  return $fmt->setSymbol($attr, $value);
}

/**
 * Set a text attribute
 *
 * @param numberformatter $fmt - NumberFormatter object.
 * @param int $attr - Attribute specifier - one of the text attribute
 *   constants.
 * @param string $value - Text for the attribute value.
 *
 * @return bool -
 */
function numfmt_set_text_attribute(NumberFormatter $fmt,
                                   $attr,
                                   $value): bool {
  return $fmt->setTextAttribute($attr, $value);
}
