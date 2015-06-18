<?hh

/**
 * A "Locale" is an identifier used to get language, culture, or
 * regionally-specific behavior from an API. PHP locales are organized and
 * identified the same way that the CLDR locales used by ICU (and many vendors
 * of Unix-like operating systems, the Mac, Java, and so forth) use. Locales
 * are identified using RFC 4646 language tags (which use hyphen, not
 * underscore) in addition to the more traditional underscore-using
 * identifiers. Unless otherwise noted the functions in this class are
 * tolerant of both formats.   Examples of identifiers include:  en-US
 * (English, United States) zh-Hant-TW (Chinese, Traditional Script, Taiwan)
 * fr-CA, fr-FR (French for Canada and France respectively)    The Locale
 * class (and related procedural functions) are used to interact with locale
 * identifiers--to verify that an ID is well-formed, valid, etc. The
 * extensions used by CLDR in UAX #35 (and inherited by ICU) are valid and
 * used wherever they would be in ICU normally.   Locales cannot be
 * instantiated as objects. All of the functions/methods provided are static.
 *  The null or empty string obtains the "root" locale. The "root" locale is
 * equivalent to "en_US_POSIX" in CLDR. Language tags (and thus locale
 * identifiers) are case insensitive. There exists a canonicalization function
 * to make case match the specification.
 */
class Locale {
  /**
   * Tries to find out best available locale based on HTTP "Accept-Language"
   * header
   *
   * @param string $header - The string containing the "Accept-Language"
   *   header according to format in RFC 2616.
   *
   * @return string - The corresponding locale identifier.
   */
  <<__Native>>
  public static function acceptFromHttp(string $header): mixed;

  /**
   * Canonicalize the locale string
   *
   * @param string $locale -
   *
   * @return string -
   */
  <<__Native>>
  public static function canonicalize(string $locale): mixed;

  /**
   * Returns a correctly ordered and delimited locale ID
   *
   * @param array $subtags - an array containing a list of key-value
   *   pairs, where the keys identify the particular locale ID subtags, and
   *   the values are the associated subtag values.   The 'variant' and
   *   'private' subtags can take maximum 15 values whereas 'extlang' can
   *   take maximum 3 values.e.g. Variants are allowed with the suffix
   *   ranging from 0-14. Hence the keys for the input array can be
   *   variant0, variant1, ...,variant14. In the returned locale id, the
   *   subtag is ordered by suffix resulting in variant0 followed by
   *   variant1 followed by variant2 and so on.   The 'variant', 'private'
   *   and 'extlang' multiple values can be specified both as array under
   *   specific key (e.g. 'variant') and as multiple numbered keys (e.g.
   *   'variant0', 'variant1', etc.).
   *
   * @return string - The corresponding locale identifier.
   */
  <<__Native>>
  public static function composeLocale(array $subtags): mixed;

  /**
   * Checks if a language tag filter matches with locale
   *
   * @param string $langtag - The language tag to check
   * @param string $locale - The language range to check against
   * @param bool $canonicalize - If true, the arguments will be converted
   *   to canonical form before matching.
   *
   * @return bool - TRUE if $locale matches $langtag FALSE otherwise.
   */
  public static function filterMatches($langtag,
                                       $locale,
                                       $canonicalize = false): bool {
    if ($locale == '*') return true;
    if (empty($locale)) $locale = self::getDefault();
    if ($canonicalize) {
      $locale  = self::canonicalize($locale);
      $langtag = self::canonicalize($langtag);
    }
    $locale  = strtolower(strtr($locale,  '-', '_'));
    $langtag = strtolower(strtr($langtag, '-', '_'));
    if ($locale == $langtag) return true;
    return ((strlen($locale) < strlen($langtag)) &&
            ($langtag[strlen($locale)] == '_') &&
            !strncmp($locale, $langtag, strlen($locale)));
  }

  /**
   * Gets the variants for the input locale
   *
   * @param string $locale - The locale to extract the variants from
   *
   * @return array - The array containing the list of all variants subtag
   *   for the locale or NULL if not present
   */
  <<__Native>>
  public static function getAllVariants(string $locale): array<string>;

  /**
   * Gets the default locale value from the INTL global 'default_locale'
   *
   * @return string - The current runtime locale
   */
  <<__Native>>
  public static function getDefault(): string;

  /**
   * Returns an appropriately localized display name for language of the
   * inputlocale
   *
   * @param string $locale - The locale to return a display language for
   * @param string $in_locale - Optional format locale to use to display
   *   the language name
   *
   * @return string - display name of the language for the $locale in the
   *   format appropriate for $in_locale.
   */
  <<__Native>>
  public static function getDisplayLanguage(string $locale,
                                            string $in_locale): string;

  /**
   * Returns an appropriately localized display name for the input locale
   *
   * @param string $locale - The locale to return a display name for.
   * @param string $in_locale - optional format locale
   *
   * @return string - Display name of the locale in the format
   *   appropriate for $in_locale.
   */
  <<__Native>>
  public static function getDisplayName(string $locale,
                                        string $in_locale): string;

  /**
   * Returns an appropriately localized display name for region of the input
   * locale
   *
   * @param string $locale - The locale to return a display region for.
   * @param string $in_locale - Optional format locale to use to display
   *   the region name
   *
   * @return string - display name of the region for the $locale in the
   *   format appropriate for $in_locale.
   */
  <<__Native>>
  public static function getDisplayRegion(string $locale,
                                          string $in_locale): string;

  /**
   * Returns an appropriately localized display name for script of the input
   * locale
   *
   * @param string $locale - The locale to return a display script for
   * @param string $in_locale - Optional format locale to use to display
   *   the script name
   *
   * @return string - Display name of the script for the $locale in the
   *   format appropriate for $in_locale.
   */
  <<__Native>>
  public static function getDisplayScript(string $locale,
                                          string $in_locale): string;

  /**
   * Returns an appropriately localized display name for variants of the input
   * locale
   *
   * @param string $locale - The locale to return a display variant for
   * @param string $in_locale - Optional format locale to use to display
   *   the variant name
   *
   * @return string - Display name of the variant for the $locale in the
   *   format appropriate for $in_locale.
   */
  <<__Native>>
  public static function getDisplayVariant(string $locale,
                                           string $in_locale): string;

  /**
   * Gets the keywords for the input locale
   *
   * @param string $locale - The locale to extract the keywords from
   *
   * @return array - Associative array containing the keyword-value pairs
   *   for this locale
   */
  <<__Native>>
  public static function getKeywords(string $locale): array<string,string>;

  /**
   * Gets the primary language for the input locale
   *
   * @param string $locale - The locale to extract the primary language
   *   code from
   *
   * @return string - The language code associated with the language or
   *   NULL in case of error.
   */
  <<__Native>>
  public static function getPrimaryLanguage(string $locale): string;

  /**
   * Gets the region for the input locale
   *
   * @param string $locale - The locale to extract the region code from
   *
   * @return string - The region subtag for the locale or NULL if not
   *   present
   */
  <<__Native>>
  public static function getRegion(string $locale): mixed;

  /**
   * Gets the script for the input locale
   *
   * @param string $locale - The locale to extract the script code from
   *
   * @return string - The script subtag for the locale or NULL if not
   *   present
   */
  <<__Native>>
  public static function getScript(string $locale): mixed;

  /**
   * Searches the language tag list for the best match to the language
   *
   * @param array $langtag - An array containing a list of language tags
   *   to compare to locale. Maximum 100 items allowed.
   * @param string $locale - The locale to use as the language range when
   *   matching.
   * @param bool $canonicalize - If true, the arguments will be converted
   *   to canonical form before matching.
   * @param string $default - The locale to use if no match is found.
   *
   * @return string - The closest matching language tag or default
   *   value.
   */
  <<__Native, __ParamCoerceModeFalse>>
  public static function lookup(array $langtag,
                                string $locale,
                                bool $canonicalize = false,
                                string $default = ""): string;

  /**
   * Returns a key-value array of locale ID subtag elements.
   *
   * @param string $locale - The locale to extract the subtag array from.
   *   Note: The 'variant' and 'private' subtags can take maximum 15 values
   *   whereas 'extlang' can take maximum 3 values.
   *
   * @return array - Returns an array containing a list of key-value
   *   pairs, where the keys identify the particular locale ID subtags, and
   *   the values are the associated subtag values. The array will be
   *   ordered as the locale id subtags e.g. in the locale id if variants
   *   are '-varX-varY-varZ' then the returned array will have
   *   variant0=varX , variant1=varY , variant2=varZ
   */
  <<__Native>>
  public static function parseLocale(string $locale): array<string,string>;

  /**
   * sets the default runtime locale
   *
   * @param string $locale - Is a BCP 47 compliant language tag
   *   containing the
   *
   * @return bool -
   */
  <<__Native>>
  public static function setDefault(string $locale): bool;

}

/**
 * Tries to find out best available locale based on HTTP "Accept-Language"
 * header
 *
 * @param string $header - The string containing the "Accept-Language"
 *   header according to format in RFC 2616.
 *
 * @return string - The corresponding locale identifier.
 */
function locale_accept_from_http(string $header): mixed {
  return Locale::acceptFromHttp($header);
}

/**
 * Canonicalize the locale string
 *
 * @param string $locale -
 *
 * @return string -
 */
function locale_canonicalize(string $locale): mixed {
  return Locale::canonicalize($locale);
}

/**
 * Returns a correctly ordered and delimited locale ID
 *
 * @param array $subtags - an array containing a list of key-value pairs,
 *   where the keys identify the particular locale ID subtags, and the
 *   values are the associated subtag values.   The 'variant' and 'private'
 *   subtags can take maximum 15 values whereas 'extlang' can take maximum
 *   3 values.e.g. Variants are allowed with the suffix ranging from 0-14.
 *   Hence the keys for the input array can be variant0, variant1,
 *   ...,variant14. In the returned locale id, the subtag is ordered by
 *   suffix resulting in variant0 followed by variant1 followed by variant2
 *   and so on.   The 'variant', 'private' and 'extlang' multiple values
 *   can be specified both as array under specific key (e.g. 'variant') and
 *   as multiple numbered keys (e.g. 'variant0', 'variant1', etc.).
 *
 * @return string - The corresponding locale identifier.
 */
function locale_compose(array $subtags) {
  return Locale::composeLocale($subtags);
}

/**
 * Checks if a language tag filter matches with locale
 *
 * @param string $langtag - The language tag to check
 * @param string $locale - The language range to check against
 * @param bool $canonicalize - If true, the arguments will be converted
 *   to canonical form before matching.
 *
 * @return bool - TRUE if $locale matches $langtag FALSE otherwise.
 */
function locale_filter_matches($langtag,
                               $locale,
                               $canonicalize = false): bool {
  return Locale::filterMatches($langtag, $locale, $canonicalize);
}

/**
 * Gets the variants for the input locale
 *
 * @param string $locale - The locale to extract the variants from
 *
 * @return array - The array containing the list of all variants subtag
 *   for the locale or NULL if not present
 */
function locale_get_all_variants(string $locale) {
  return Locale::getAllVariants($locale);
}

/**
 * Gets the default locale value from the INTL global 'default_locale'
 *
 * @return string - The current runtime locale
 */
function locale_get_default(): string {
  return Locale::getDefault();
}

/**
 * Returns an appropriately localized display name for language of the
 * inputlocale
 *
 * @param string $locale - The locale to return a display language for
 * @param string $in_locale - Optional format locale to use to display
 *   the language name
 *
 * @return string - display name of the language for the $locale in the
 *   format appropriate for $in_locale.
 */
function locale_get_display_language(string $locale,
                                     string $in_locale): string {
  return Locale::getDisplayLanguage($locale, $in_locale);
}

/**
 * Returns an appropriately localized display name for the input locale
 *
 * @param string $locale - The locale to return a display name for.
 * @param string $in_locale - optional format locale
 *
 * @return string - Display name of the locale in the format appropriate
 *   for $in_locale.
 */
function locale_get_display_name(string $locale,
                                 string $in_locale): string {
  return Locale::getDisplayName($locale, $in_locale);
}

/**
 * Returns an appropriately localized display name for region of the input
 * locale
 *
 * @param string $locale - The locale to return a display region for.
 * @param string $in_locale - Optional format locale to use to display
 *   the region name
 *
 * @return string - display name of the region for the $locale in the
 *   format appropriate for $in_locale.
 */
function locale_get_display_region(string $locale,
                                   string $in_locale): string {
  return Locale::getDisplayRegion($locale, $in_locale);
}

/**
 * Returns an appropriately localized display name for script of the input
 * locale
 *
 * @param string $locale - The locale to return a display script for
 * @param string $in_locale - Optional format locale to use to display
 *   the script name
 *
 * @return string - Display name of the script for the $locale in the
 *   format appropriate for $in_locale.
 */
function locale_get_display_script(string $locale,
                                   string $in_locale): string {
  return Locale::getDisplayScript($locale, $in_locale);
}

/**
 * Returns an appropriately localized display name for variants of the input
 * locale
 *
 * @param string $locale - The locale to return a display variant for
 * @param string $in_locale - Optional format locale to use to display
 *   the variant name
 *
 * @return string - Display name of the variant for the $locale in the
 *   format appropriate for $in_locale.
 */
function locale_get_display_variant(string $locale,
                                    string $in_locale): string {
  return Locale::getDisplayVariant($locale, $in_locale);
}

/**
 * Gets the keywords for the input locale
 *
 * @param string $locale - The locale to extract the keywords from
 *
 * @return array - Associative array containing the keyword-value pairs
 *   for this locale
 */
function locale_get_keywords(string $locale) {
  return Locale::getKeywords($locale);
}

/**
 * Gets the primary language for the input locale
 *
 * @param string $locale - The locale to extract the primary language
 *   code from
 *
 * @return string - The language code associated with the language or
 *   NULL in case of error.
 */
function locale_get_primary_language(string $locale): string {
  return Locale::getPrimaryLanguage($locale);
}

/**
 * Gets the region for the input locale
 *
 * @param string $locale - The locale to extract the region code from
 *
 * @return string - The region subtag for the locale or NULL if not
 *   present
 */
function locale_get_region(string $locale): mixed {
  return Locale::getRegion($locale);
}

/**
 * Gets the script for the input locale
 *
 * @param string $locale - The locale to extract the script code from
 *
 * @return string - The script subtag for the locale or NULL if not
 *   present
 */
function locale_get_script(string $locale): mixed {
  return Locale::getScript($locale);
}

/**
 * Searches the language tag list for the best match to the language
 *
 * @param array $langtag - An array containing a list of language tags to
 *   compare to locale. Maximum 100 items allowed.
 * @param string $locale - The locale to use as the language range when
 *   matching.
 * @param bool $canonicalize - If true, the arguments will be converted
 *   to canonical form before matching.
 * @param string $default - The locale to use if no match is found.
 *
 * @return string - The closest matching language tag or default value.
 */
function locale_lookup(array $langtag,
                       string $locale,
                       bool $canonicalize = false,
                       string $default): string {
  return Locale::lookup($langtag, $locale, $canonicalize, $default);
}

/**
 * Returns a key-value array of locale ID subtag elements.
 *
 * @param string $locale - The locale to extract the subtag array from.
 *   Note: The 'variant' and 'private' subtags can take maximum 15 values
 *   whereas 'extlang' can take maximum 3 values.
 *
 * @return array - Returns an array containing a list of key-value pairs,
 *   where the keys identify the particular locale ID subtags, and the
 *   values are the associated subtag values. The array will be ordered as
 *   the locale id subtags e.g. in the locale id if variants are
 *   '-varX-varY-varZ' then the returned array will have variant0=varX ,
 *   variant1=varY , variant2=varZ
 */
function locale_parse(string $locale): array<string,string> {
  return Locale::parseLocale($locale);
}

/**
 * sets the default runtime locale
 *
 * @param string $locale - Is a BCP 47 compliant language tag containing
 *   the
 *
 * @return bool -
 */
function locale_set_default(string $locale): bool {
  return Locale::setDefault($locale);
}
