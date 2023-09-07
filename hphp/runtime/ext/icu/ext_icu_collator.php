<?hh // partial

/**
 * Provides string comparison capability with support for appropriate
 * locale-sensitive sort orderings.
 */
<<__NativeData>>
class Collator {
  /**
   * Construct a new Collator instance
   */
  <<__Native>>
  public function __construct(string $locale): void;

  /**
   * Sort array maintaining index association
   *
   * @param array $arr - Array of strings to sort.
   * @param int $sort_flag - Optional sorting type, one of the following:
   *      Collator::SORT_REGULAR - compare items normally (don't change
   *   types)     Collator::SORT_NUMERIC - compare items numerically
   *   Collator::SORT_STRING - compare items as strings      Default
   *   $sort_flag value is Collator::SORT_REGULAR. It is also used if an
   *   invalid $sort_flag value has been specified.
   *
   * @return bool -
   */
  <<__Native>>
  public function asort(inout mixed $arr,
                        int $sort_flag = Collator::SORT_REGULAR): bool;

  /**
   * Compare two Unicode strings
   *
   * @param string $str1 - The first string to compare.
   * @param string $str2 - The second string to compare.
   *
   * @return int - Return comparison result:     1 if str1 is greater
   *   than str2 ;     0 if str1 is equal to str2;     -1 if str1 is less
   *   than str2 .    On error boolean FALSE is returned.
   */
  <<__Native>>
  public function compare(mixed $str1,
                          mixed $str2): mixed;

  /**
   * Create a Collator
   *
   * @param string $locale - The locale containing the required collation
   *   rules. Special values for locales can be passed in - if null is
   *   passed for the locale, the default locale collation rules will be
   *   used. If empty string ("") or "root" are passed, UCA rules will be
   *   used.
   *
   * @return Collator - Return new instance of Collator object, or NULL
   *   on error.
   */
  public static function create(string $locale): Collator {
    return new Collator($locale);
  }

  /**
   * Get collation attribute value
   *
   * @param int $attr - Attribute to get value for.
   *
   * @return int - Attribute value, or boolean FALSE on error.
   */
  <<__Native>>
  public function getAttribute(int $attr): int;

  /**
   * Get collator's last error code
   *
   * @return int - Error code returned by the last Collator API function
   *   call.
   */
  <<__Native>>
  public function getErrorCode(): int;

  /**
   * Get text for collator's last error code
   *
   * @return string - Description of an error occurred in the last
   *   Collator API function call.
   */
  <<__Native>>
  public function getErrorMessage(): string;

  /**
   * Get the locale name of the collator
   *
   * @param int $type - You can choose between valid and actual locale (
   *   Locale::VALID_LOCALE and Locale::ACTUAL_LOCALE, respectively).
   *
   * @return string - Real locale name from which the collation data
   *   comes. If the collator was instantiated from rules or an error
   *   occurred, returns boolean FALSE.
   */
  <<__Native>>
  public function getLocale(int $type): string;

  /**
   * Get sorting key for a string
   *
   * @param string $str - The string to produce the key from.
   *
   * @return string - Returns the collation key for the string. Collation
   *   keys can be compared directly instead of strings.
   */
  <<__Native>>
  public function getSortKey(string $str): mixed;

  /**
   * Get current collation strength
   *
   * @return int - Returns current collation strength, or boolean FALSE
   *   on error.
   */
  <<__Native>>
  public function getStrength(): int;

  /**
   * Set collation attribute
   *
   * @param int $attr - Attribute.
   * @param int $val - Attribute value.
   *
   * @return bool -
   */
  <<__Native>>
  public function setAttribute(int $attr,
                               int $val): bool;

  /**
   * Set collation strength
   *
   * @param int $strength - Strength to set.  Possible values are:
   *   Collator::PRIMARY     Collator::SECONDARY     Collator::TERTIARY
   *   Collator::QUATERNARY     Collator::IDENTICAL
   *   Collator::DEFAULT_STRENGTH
   *
   * @return bool -
   */
  <<__Native>>
  public function setStrength(int $strength): bool;

  /**
   * Sort array using specified collator and sort keys
   *
   * @param array $arr - Array of strings to sort
   *
   * @return bool -
   */
  <<__Native>>
  public function sortWithSortKeys(inout mixed $arr): bool;

  /**
   * Sort array using specified collator
   *
   * @param array $arr - Array of strings to sort.
   * @param int $sort_flag - Optional sorting type, one of the following:
   *        Collator::SORT_REGULAR - compare items normally (don't change
   *   types)     Collator::SORT_NUMERIC - compare items numerically
   *   Collator::SORT_STRING - compare items as strings    Default sorting
   *   type is Collator::SORT_REGULAR. It is also used if an invalid
   *   sort_flag value has been specified.
   *
   * @return bool -
   */
  <<__Native>>
  public function sort(inout mixed $arr,
                       int $sort_flag = Collator::SORT_REGULAR): bool;
}

/**
 * Sort array maintaining index association
 *
 * @param Collator $coll - Collator object.
 * @param array $arr - Array of strings to sort.
 * @param int $sort_flag - Optional sorting type, one of the following:
 *    Collator::SORT_REGULAR - compare items normally (don't change types)
 *      Collator::SORT_NUMERIC - compare items numerically
 *   Collator::SORT_STRING - compare items as strings      Default
 *   $sort_flag value is Collator::SORT_REGULAR. It is also used if an
 *   invalid $sort_flag value has been specified.
 *
 * @return bool -
 */
function collator_asort(Collator $coll,
                        inout mixed $arr,
                        int $sort_flag = Collator::SORT_REGULAR): bool {
  return $coll->asort(inout $arr, $sort_flag);
}

/**
 * Compare two Unicode strings
 *
 * @param Collator $coll - Collator object.
 * @param string $str1 - The first string to compare.
 * @param string $str2 - The second string to compare.
 *
 * @return int - Return comparison result:     1 if str1 is greater than
 *   str2 ;     0 if str1 is equal to str2;     -1 if str1 is less than
 *   str2 .    On error boolean FALSE is returned.
 */
function collator_compare(Collator $coll,
                          mixed $str1,
                          mixed $str2): int {
  return $coll->compare($str1, $str2);
}

/**
 * Create a collator
 *
 * @param string $locale - The locale containing the required collation
 *   rules. Special values for locales can be passed in - if null is passed
 *   for the locale, the default locale collation rules will be used. If
 *   empty string ("") or "root" are passed, UCA rules will be used.
 *
 * @return Collator - Return new instance of Collator object, or NULL on
 *   error.
 */
function collator_create(string $locale): Collator {
  return Collator::create($locale);
}

/**
 * Get collation attribute value
 *
 * @param Collator $coll - Collator object.
 * @param int $attr - Attribute to get value for.
 *
 * @return int - Attribute value, or boolean FALSE on error.
 */
function collator_get_attribute(Collator $coll,
                                int $attr): int {
  return $coll->getAttribute($attr);
}

/**
 * Get collator's last error code
 *
 * @param Collator $coll - Collator object.
 *
 * @return int - Error code returned by the last Collator API function
 *   call.
 */
function collator_get_error_code(Collator $coll): int {
  return $coll->getErrorCode();
}

/**
 * Get text for collator's last error code
 *
 * @param Collator $coll - Collator object.
 *
 * @return string - Description of an error occurred in the last Collator
 *   API function call.
 */
function collator_get_error_message(Collator $coll): string {
  return $coll->getErrorMessage();
}

/**
 * Get the locale name of the collator
 *
 * @param Collator $coll - Collator object.
 * @param int $type - You can choose between valid and actual locale (
 *   Locale::VALID_LOCALE and Locale::ACTUAL_LOCALE, respectively).
 *
 * @return string - Real locale name from which the collation data comes.
 *   If the collator was instantiated from rules or an error occurred,
 *   returns boolean FALSE.
 */
function collator_get_locale(Collator $coll,
                             int $type): string {
  return $coll->getLocale($type);
}

/**
 * Get sorting key for a string
 *
 * @param Collator $coll - Collator object.
 * @param string $str - The string to produce the key from.
 *
 * @return string - Returns the collation key for the string. Collation
 *   keys can be compared directly instead of strings.
 */
function collator_get_sort_key(Collator $coll,
                               string $str): string {
  return $coll->getSortKey($str);
}

/**
 * Get current collation strength
 *
 * @param Collator $coll - Collator object.
 *
 * @return int - Returns current collation strength, or boolean FALSE on
 *   error.
 */
function collator_get_strength(Collator $coll): int {
  return $coll->getStrength();
}

/**
 * Set collation attribute
 *
 * @param Collator $coll - Collator object.
 * @param int $attr - Attribute.
 * @param int $val - Attribute value.
 *
 * @return bool -
 */
function collator_set_attribute(Collator $coll,
                                int $attr,
                                int $val): bool {
  return $coll->setAttribute($attr, $val);
}

/**
 * Set collation strength
 *
 * @param Collator $coll - Collator object.
 * @param int $strength - Strength to set.  Possible values are:
 *   Collator::PRIMARY     Collator::SECONDARY     Collator::TERTIARY
 *   Collator::QUATERNARY     Collator::IDENTICAL
 *   Collator::DEFAULT_STRENGTH
 *
 * @return bool -
 */
function collator_set_strength(Collator $coll,
                               int $strength): bool {
  return $coll->setStrength($strength);
}

/**
 * Sort array using specified collator and sort keys
 *
 * @param Collator $coll - Collator object.
 * @param array $arr - Array of strings to sort
 *
 * @return bool -
 */
function collator_sort_with_sort_keys(Collator $coll,
                                      inout mixed $arr): bool {
  return $coll->sortWithSortKeys(inout $arr);
}

/**
 * Sort array using specified collator
 *
 * @param Collator $coll - Collator object.
 * @param array $arr - Array of strings to sort.
 * @param int $sort_flag - Optional sorting type, one of the following:
 *      Collator::SORT_REGULAR - compare items normally (don't change
 *   types)     Collator::SORT_NUMERIC - compare items numerically
 *   Collator::SORT_STRING - compare items as strings    Default sorting
 *   type is Collator::SORT_REGULAR. It is also used if an invalid
 *   sort_flag value has been specified.
 *
 * @return bool -
 */
function collator_sort(Collator $coll,
                       inout mixed $arr,
                       int $sort_flag = Collator::SORT_REGULAR): bool {
  return $coll->sort(inout $arr, $sort_flag);
}
