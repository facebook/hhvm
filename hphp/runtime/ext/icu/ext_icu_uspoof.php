<?hh

/**
 * Unicode Security and Spoofing Detection
 * http://icu-project.org/apiref/icu4c/uspoof_8h.html
 */
<<__NativeData>>
class SpoofChecker {
  /**
   * Creates a spoof checker that checks for visually confusing
   * characters in a string.  By default, runs the all tests but
   * SINGLE_SCRIPT
   */
  public function __construct() {}

  /**
   * Check the specified UTF-8 string for possible
   * security or spoofing issues.
   *
   * @param string $text - A UTF-8 string to be checked for
   *                       possible security issues.
   * @param inout mixed $issuesFound - If passed, this will hold an integer
   *                                   value with bits set for any potential
   *                                   security or spoofing issues detected.
   *                                   Zero is returned if no issues are found
   *                                   with the input string.
   *
   * @return bool - Returns TRUE if the string has possible security or
   *                spoofing issues, FALSE otherwise.
   */
  <<__Native>>
  public function isSuspicious(string $text,
                               <<__OutOnly("KindOfInt64")>>
                               inout mixed $issuesFound): bool;

  /**
   * Check the whether two specified UTF-8 strings are visually confusable.
   * The types of confusability to be tested - single script, mixed script,
   * or whole script - are determined by the check options set for this
   * instance.
   *
   * @param string $s1 - The first of the two UTF-8 strings to be
   *                     compared for confusability.
   * @param string $s2 - The second of the two UTF-8 strings to be
   *                     compared for confusability.
   * @param inout mixed $issuesFound - If passed, this will hold an integer
   *                                   value with bits set for any potential
   *                                   security or spoofing issues detected.
   *                                   Zero is returned if no issues are found
   *                                   with the input string.
   *
   * @return bool - Returns TRUE if the two strings are confusable,
   *                FALSE otherwise.
   */
  <<__Native>>
  public function areConfusable(string $s1, string $s2,
                                <<__OutOnly("KindOfInt64")>>
                                inout mixed $issuesFound): bool;

  /**
   * Limit characters that are acceptable in identifiers being checked to those
   * normally used with the languages associated with the specified locales.
   *
   * @param string $localesList - A list of locales, from which the language
   *                              and associated script are extracted.
   *                              The locales are comma-separated if there is
   *                              more than one. White space may not appear
   *                              within an individual locale, but is ignored
   *                              otherwise.
   *                              The locales are syntactically like those from
   *                              the HTTP Accept-Language header.
   *                              If the localesList is empty, no restrictions
   *                              will be placed on the allowed characters.
   */
  <<__Native>>
  public function setAllowedLocales(string $localesList): void;

  /**
   * Specify the set of checks that will be performed by the check function.
   *
   * @param int $checks - Set of checks that this spoof checker will perform.
   *                      The value is a bit set, obtained by OR-ing together
   *                      the constant values in this class.
   */
  <<__Native>>
  public function setChecks(int $checks): void;
}
