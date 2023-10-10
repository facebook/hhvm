<?hh
/**
 * Generates localized date and/or time format pattern strings suitable for use
 * in IntlDateFormatter.
 *
 * Transforms unordered skeleton formats like "MMddyyyy" to use the correct
 * ordering and separators for the locale (for example, one locale might use
 * "dd-MM-yyyy" when another uses "yyyy/MM/dd").
 *
 * See Unicode UTS #35 appendix F (Date Format Patterns) for valid input format
 * patterns:
 * http://unicode.org/reports/tr35/tr35-6.html#Date_Format_Patterns
 *
 * Example usage:
 * $locale = 'en_US';
 * $generator = IntlDatePatternGenerator::createInstance($locale);
 * $pattern = $generator->getBestPattern('MMddyyyy');
 * $formatter = IntlDateFormatter::create($locale, null, null);
 * $formatter->setPattern($pattern);
 * $date = $formatter->format(new DateTime());
 *
 * Constants:
 *
 * Pattern fields:
 *   IntlDatePatternGenerator::ERA_PATTERN_FIELD
 *   IntlDatePatternGenerator::YEAR_PATTERN_FIELD
 *   IntlDatePatternGenerator::QUARTER_PATTERN_FIELD
 *   IntlDatePatternGenerator::MONTH_PATTERN_FIELD
 *   IntlDatePatternGenerator::WEEK_OF_YEAR_PATTERN_FIELD
 *   IntlDatePatternGenerator::WEEK_OF_MONTH_PATTERN_FIELD
 *   IntlDatePatternGenerator::WEEKDAY_PATTERN_FIELD
 *   IntlDatePatternGenerator::DAY_OF_YEAR_PATTERN_FIELD
 *   IntlDatePatternGenerator::DAY_OF_WEEK_IN_MONTH_PATTERN_FIELD
 *   IntlDatePatternGenerator::DAY_PATTERN_FIELD
 *   IntlDatePatternGenerator::DAYPERIOD_PATTERN_FIELD
 *   IntlDatePatternGenerator::HOUR_PATTERN_FIELD
 *   IntlDatePatternGenerator::MINUTE_PATTERN_FIELD
 *   IntlDatePatternGenerator::SECOND_PATTERN_FIELD
 *   IntlDatePatternGenerator::FRACTIONAL_SECOND_PATTERN_FIELD
 *   IntlDatePatternGenerator::ZONE_PATTERN_FIELD
 *
 * Pattern conflict status:
 *   IntlDatePatternGenerator::PATTERN_NO_CONFLICT
 *   IntlDatePatternGenerator::PATTERN_BASE_CONFLICT
 *   IntlDatePatternGenerator::PATTERN_CONFLICT
 *
 */
<<__NativeData>>
class IntlDatePatternGenerator {

  /**
   * Private constructor for disallowing instantiation
   *
   * @return  -
   */
  private function __construct(): void {}

  /**
   * Creates a flexible generator according to the data for a given locale.
   *
   * @param string $locale - Data will be loaded into the generator for the
   *   locale specified.
   *
   * @throws Exception - If there is an error creating the generator,
   *   an exception will be thrown.
   *
   * @return IntlDatePatternGenerator
   */
  <<__Native>>
  public static function createInstance(string $locale):
    IntlDatePatternGenerator;

  /**
   * Creates an empty generator, to be constructed with addPattern(...) etc.
   *
   * @throws Exception - If there is an error creating the generator,
   *   an exception will be thrown.
   *
   * @return IntlDatePatternGenerator
   */
  <<__Native>>
  public static function createEmptyInstance():
    IntlDatePatternGenerator;

  /**
   * Utility to return a unique skeleton from a given pattern.
   * For example, both "MMM-dd" and "dd/MMM" produce the skeleton "MMMdd".
   *
   * @param string $pattern - Input pattern, such as "dd/MMM"
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return string - skeleton such as "MMMdd"
   */
  <<__Native>>
  public function getSkeleton(string $pattern): string;

  /**
   * Utility to return a unique base skeleton from a given pattern.
   * This is the same as the skeleton, except that differences in length are
   * minimized so as to only preserve the difference between string and numeric
   * form. So for example, both "MMM-dd" and "d/MMM" produce the skeleton "MMMd"
   * (notice the single d).
   *
   * @param string $pattern - Input pattern, such as "dd/MMM"
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return string - base skeleton, such as "Md"
   */
  <<__Native>>
  public function getBaseSkeleton(string $pattern): string;

  /**
   * Adds a pattern to the generator.
   * If the pattern has the same skeleton as an existing pattern, and the
   * override parameter is set, then the previous value is overridden.
   * Otherwise, the previous value is retained.
   * Note that single-field patterns (like "MMM") are automatically added, and
   * don't need to be added explicitly!
   *
   * @param string $pattern - Input pattern, such as "dd/MMM"
   * @param bool $override - When existing values are to be overridden use true,
   *   otherwise use false.
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return int - pattern conflict status (see constants)
   */
  <<__Native>>
  public function addPattern(string $pattern, bool $override): int;

  /**
   * An append item format is a pattern used to append a field if there is no
   * good match.
   *
   * For example, suppose that the input skeleton is "GyyyyMMMd", and there is
   * no matching pattern internally, but there is a pattern matching "yyyyMMMd",
   * say "d-MM-yyyy". Then that pattern is used, plus the G. The way these two
   * are conjoined is by using the AppendItemFormat for G (era). So if that
   * value is, say "{0}, {1}" then the final resulting pattern is
   * "d-MM-yyyy, G".
   *
   * There are actually three available variables: {0} is the pattern so far,
   * {1} is the element we are adding, and {2} is the name of the element.
   *
   * @param int $field - Pattern field (see constants)
   * @param string $value - Pattern, such as "{0}, {1}"
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   */
  <<__Native>>
  public function setAppendItemFormat(int $field, string $value): void;

  /**
   * @param int $field - Pattern field (see constants)
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return string - Append item format for the given pattern field
   */
  <<__Native>>
  public function getAppendItemFormat(int $field): string;

  /**
   * Sets the name of a field, eg "era" in English for ERA.
   * These are only used if the corresponding AppendItemFormat is used, and if
   * it contains a {2} variable.
   *
   * @param int $field - Pattern field (see constants)
   * @param string $name - Name of the field
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   */
  <<__Native>>
  public function setAppendItemName(int $field, string $name): void;

  /**
   * @param int $field - Pattern field (see constants)
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return string - Append item name for the given pattern field
   */
  <<__Native>>
  public function getAppendItemName(int $field): string;

  /**
   * The date time format is a message format pattern used to compose date and
   * time patterns.
   *
   * The default value is "{0} {1}", where {0} will be replaced by the date
   * pattern and {1} will be replaced by the time pattern.
   *
   * This is used when the input skeleton contains both date and time fields,
   * but there is not a close match among the added patterns. For example,
   * suppose that this object was created by adding "dd-MMM" and "hh:mm", and
   * its datetimeFormat is the default "{0} {1}". Then if the input skeleton is
   * "MMMdhmm", there is not an exact match, so the input skeleton is broken up
   * into two components "MMMd" and "hmm". There are close matches for those two
   * skeletons, so the result is put together with this pattern, resulting in
   * "d-MMM h:mm".
   *
   * @param string $dateTimeFormat - Format pattern. {0} replaced by the date
   *   pattern, {1} replaced by the time pattern.
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   */
  <<__Native>>
  public function setDateTimeFormat(string $dateTimeFormat): void;

  /**
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return string - The date time format, for example: "{0} {1}"
   */
  <<__Native>>
  public function getDateTimeFormat(): string;

  /**
   * Returns the best pattern matching the input skeleton.
   * It is guaranteed to have all of the fields in the skeleton.
   *
   * @param string $skeleton - The skeleton is a pattern containing only the
   *   variable fields. For example, "MMMdd" and "mmhh" are skeletons.
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return string - The best pattern found for the given skeleton
   */
  <<__Native>>
  public function getBestPattern(string $skeleton): string;

  /**
   * Adjusts the field types (width and subtype) of a pattern to match what is
   * in a skeleton.
   *
   * Example: given an input pattern of "d-M H:m", and a skeleton of
   * "MMMMddhhmm", the output pattern will be "dd-MMMM hh:mm".
   *
   * @param string $pattern - Input pattern
   * @param string $skeleton - The skeleton is a pattern containing only the
   *   variable fields. For example, "MMMdd" and "mmhh" are skeletons.
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return string - Pattern adjusted to match the skeleton fields widths and
   *   subtypes.
   */
  <<__Native>>
  public function replaceFieldTypes(string $pattern, string $skeleton): string;
  /**
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return IntlIterator - Contains all the skeletons (in canonical form).
   */
  <<__Native>>
  public function getSkeletons(): IntlIterator;

  /**
   * Get the pattern corresponding to a given skeleton.
   *
   * @param string $skeleton - The skeleton is a pattern containing only the
   *   variable fields. For example, "MMMdd" and "mmhh" are skeletons.
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return string - pattern
   */
  <<__Native>>
  public function getPatternForSkeleton(string $skeleton): string;

  /**
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return IntlIterator - Contains all the base skeletons (in canonical form).
   */
  <<__Native>>
  public function getBaseSkeletons(): IntlIterator;

  /**
   * The decimal value is used in formatting fractions of seconds.
   *
   * If the skeleton contains fractional seconds, then this is used with the
   * fractional seconds. For example, suppose that the input pattern is
   * "hhmmssSSSS", and the best matching pattern internally is "H:mm:ss", and
   * the decimal string is ",". Then the resulting pattern is modified to be
   * "H:mm:ss,SSSS"
   *
   * @param string $decimal - The string to represent the decimal
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   */
  <<__Native>>
  public function setDecimal(string $decimal): void;

  /**
   * The decimal value is used in formatting fractions of seconds.
   *
   * @throws Exception - If there is an error, an exception will be thrown.
   *
   * @return string - The string representing the decimal (normally ".")
   */
  <<__Native>>
  public function getDecimal(): string;

  /**
   * Get last error code on the object
   *
   * @return int - An ICU error code indicating either success, failure
   *   or a warning.
   */
  <<__Native>>
  public function getErrorCode(): int;

  /**
   * Get last error message on the object
   *
   * @return string - The error message associated with last error that
   *   occurred in a function call on this object, or a string indicating
   *   the non-existence of an error.
   */
  <<__Native>>
  public function getErrorMessage(): string;

}
