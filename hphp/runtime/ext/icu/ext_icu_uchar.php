<?hh

class IntlChar {
  /**
   * Similar to \chr(), returns a UTF-8 encoded codepoint
   * When passed a single UTF-8 codepoint as a string,
   *   acts as an identity validator function
   *
   * @param (int|string) $cp - Codepoint as an integer or UTF-8 string
   * @return string - UTF-8 encoded codepoint
   */
  <<__Native, __IsFoldable>>
  public static function chr(mixed $cp): mixed;

  /**
   * Similar to \ord(), returns an integer character value
   * When passed an integer, acts as an identity validator
   *
   * @param (int|string) $cp - Codepoint as an integer or UTf-8 string
   * @return int - Unicode character number
   */
  <<__Native, __IsFoldable>>
  public static function ord(mixed $cp): mixed;

  /**
   * Whether or not the specified codepoint has the noted boolean property.
   *
   * @param (int|string) $cp - Codepoint as an integer or UTf-8 string
   * @param int $prop - One of IntlChar::PROPERTY_* constants
   * @return bool - Whether or not the codepoint has the property
   */
  <<__Native, __IsFoldable>>
  public static function hasBinaryProperty(mixed $cp, int $prop): mixed;

  /**
   * The codepoint specific integer property value.
   *
   * @param (int|string) $cp - Codepoint as an integer or UTf-8 string
   * @param int $prop - One of IntlChar::PROPERTY_* constants
   * @return int - Value of the property for this codepoint
   */
  <<__Native, __IsFoldable>>
  public static function getIntPropertyValue(mixed $cp, int $prop): mixed;

  /**
   * The maximum value a given property can have for any character
   *
   * @param int $prop - One of IntlChar::PROPERTY_* constants
   * @return int - Maximum value for the property specified
   */
  <<__Native, __IsFoldable>>
  public static function getIntPropertyMaxValue(int $prop): int;

  /**
   * The minimum value a given property can have for any character
   *
   * @param int $prop - One of IntlChar::PROPERTY_* constants
   * @return int - Minimum value for the property specified
   */
  <<__Native, __IsFoldable>>
  public static function getIntPropertyMinValue(int $prop): int;

  /**
   * Get the numeric value for a Unicode codepoint.
   *
   * @param (int|string) $cp - The codepoint to get a numeric value for
   * @return float - The numeric value or IntlChar::NO_NUMERIC_VALUE
   */
  <<__Native, __IsFoldable>>
  public static function getNumericValue(mixed $cp): mixed;

  /**
   * Iteratively call a handler function with blocks of contiguous
   *   character types.
   *
   * @param callback $cb
   *   function (
   *     int $start, // First codepoint in this block
   *     int $limit, // One past the last codepoint of this block
   *     int $type,  // The type of codepoints in this block
   *                 // One of IntlChar::CHAR_CATEGORY_* constants
   *   )
   */
  <<__Native>>
  public static function enumCharTypes(
    (function(int, int, int): void) $cb,
  ): void;

  /**
   * Get the block which this codepoint belongs to
   *
   * @param (int|string) $cp - The codepoint to check
   * @return int - The Unicode block (See IntlChar::BLOCK_CODE_*)
   */
  <<__Native, __IsFoldable>>
  public static function getBlockCode(mixed $cp): mixed;

  /**
   * Get the formal name for a given codepoint
   *
   * @param (int|string) $cp - The codepoint to check
   * @param int $choice - The name type (e.g. extended, alias, etc...)
   * @return string - The name of the unicode point
   */
  <<__Native, __IsFoldable>>
  public static function charName(
    mixed $cp,
    int $choice = IntlChar::UNICODE_CHAR_NAME,
  ): mixed;

  /**
   * Translate a formal character name to a codepoint value
   *
   * @param string $name - Name of a codepoint (i.e. "LATIN SMALL LETTER S")
   * @param int $choice - The name type (e.g. extended, alias, etc...)
   * @return int - The unicode codepoint for the named character
   */
  <<__Native, __IsFoldable>>
  public static function charFromName(
    string $name,
    int $choice = IntlChar::UNICODE_CHAR_NAME,
  ): mixed;

  /**
   * Enumerate all named characters
   *
   * @param (int|string) $start - The first codepoint to begin iterating from
   * @param (int|string) $limit - The codepoint after the last codepoint
   * @param callable $cb
   *   function (
   *     int $cp,      // A codepoint
   *     int $choice,  // The name type (e.g. extended, alias, etc...)
   *     string $name, // The name of the character
   *   )
   * @param int $choice - The name type (e.g. extended, alias, etc...)
   */
  <<__Native>>
  public static function enumCharNames(
    mixed $start,
    mixed $limit,
    (function(int, int, string): void) $cb,
    int $choice = IntlChar::UNICODE_CHAR_NAME,
  ): void;

  /**
   * Return the Unicode name for a given property.
   *
   * @param int $prop - IntlChar::PROPERTY_*
   * @param int $choice - Name choice
   * @return string - The property name
   */
  <<__Native, __IsFoldable>>
  public static function getPropertyName(
    int $prop,
    int $choice = IntlChar::LONG_PROPERTY_NAME,
  ): mixed;

  /**
   * Return the Unicode name for a given property value.
   *
   * @param int $prop - IntlChar::PROPERTY_*
   * @param int $value - Property specific value
   * @param int $choice - Name choice
   * @return string - The Property Value name
   */
  <<__Native, __IsFoldable>>
  public static function getPropertyValueName(
    int $prop,
    int $value,
    int $choice = IntlChar::LONG_PROPERTY_NAME,
  ): mixed;

  /**
   * Look up the property enum value associated with a property name
   *
   * @param string $alias - Name of a character property
   * @return int - Property enum value
   */
  <<__Native, __IsFoldable>>
  public static function getPropertyEnum(string $alias): int;

  /**
   * Look up the property value enum associated with a
   *   specific value associated with a known property
   *
   * @param int $prop - The property, one of IntlCHar::PROPERTY_*
   * @param string $name - The name of the property value
   * @return int - The enum for that property/value combination
   */
  <<__Native, __IsFoldable>>
  public static function getPropertyValueEnum(int $prop, string $name): int;

  /**
   * Fold the case for the given codepoint
   *
   * @param (int|string) $cp - Codepoint to fold
   * @param int $options - How to fold it
   * @return (int|string) - The folded codepoint
   */
  <<__Native, __IsFoldable>>
  public static function foldCase(
    mixed $cp,
    int $options = IntlChar::FOLD_CASE_DEFAULT,
  ): mixed;

  /**
   * Map a unicode codepoint to a numeric digit value
   *   i.e. 0x31 => int(1)
   *
   * @param (int|string) $cp - The codepoint to get the numeric value of
   * @param int $radix - Base to translate the value in
   * @return int - Numeric value for the digit character
   */
  <<__Native, __IsFoldable>>
  public static function digit(mixed $cp, int $radix = 10): mixed;

  /**
   * Map a numeric digit to a unicode codepoint
   *   i.e. int(1) => 0x31
   *
   * @param int $digit - Numeric value
   * @param int $radix - Base to translate the value in
   * @return int - Codepoint representing $digit in $radix
   */
  <<__Native, __IsFoldable>>
  public static function forDigit(int $digit, int $radix = 10): int;

  /**
   * Get the "age" of the code point.
   *
   * @param (int|string) $cp - Codepoint to introspect
   * @return array<int,int> - 4 octet version ID
   */
  <<__Native, __IsFoldable>>
  public static function charAge(mixed $cp): mixed;

  /**
   * Gets the Unicode version information.
   *
   * @return array - Current Unicode database version
   */
  <<__Native, __IsFoldable>>
  public static function getUnicodeVersion(): varray;

  /**
   * Get the FC_NFKC_Closure property string for a character.
   *
   * @param (int|string) $cp - Codepoint to get property for
   * @return string - FC_NFKC_Closure property for given $cp
   */
  <<__Native, __IsFoldable>>
  public static function getFC_NFKC_Closure(mixed $cp): mixed;

  /**
   * Check if a code point has the Alphabetic Unicode property.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isUAlphabetic(mixed $cp): mixed;

  /**
   * Check if a code point has the Lowercase Unicode property.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isULowercase(mixed $cp): mixed;

  /**
   * Check if a code point has the Uppercase Unicode property.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isUUppercase(mixed $cp): mixed;

  /**
   *Check if a code point has the White_Space Unicode property.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isUWhiteSpace(mixed $cp): mixed;

  /**
   * Determines whether the specified code point has the
   *   general category "Ll" (lowercase letter).
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function islower(mixed $cp): mixed;

  /**
   * Determines whether the specified code point has the
   *   general category "Lu" (uppercase letter).
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isupper(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is a titlecase letter.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function istitle(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is a
   *   digit character according to Java.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isdigit(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is a letter character.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isalpha(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is an alphanumeric
   *   character (letter or digit) according to Java.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isalnum(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is a hexadecimal digit.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isxdigit(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is a punctuation character.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function ispunct(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is a "graphic" character
   *   (printable, excluding spaces).
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isgraph(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is a "blank" or
   *   "horizontal space", a character that visibly separates words on a line.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isblank(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is "defined",
   *   which usually means that it is assigned a character.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isdefined(mixed $cp): mixed;

  /**
   * Determines if the specified character is a space character or not.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isspace(mixed $cp): mixed;

  /**
   * Determine if the specified code point is a space character
   *   according to Java.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isJavaSpaceChar(mixed $cp): mixed;

  /**
   * Determines if the specified code point is a whitespace character
   *   according to Java/ICU.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isWhitespace(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is a control character
   *   (as defined by this function).
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function iscntrl(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is an ISO control code.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isISOControl(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is a printable character.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isprint(mixed $cp): mixed;

  /**
   * Determines whether the specified code point is a base character.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isbase(mixed $cp): mixed;

  /**
   * Determines whether the code point has the Bidi_Mirrored property.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isMirrored(mixed $cp): mixed;

  /**
   * Determines if the specified character is permissible as the
   *   first character in an identifier according to Unicode
   *   (The Unicode Standard, Version 3.0, chapter 5.16 Identifiers).
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isIDStart(mixed $cp): mixed;

  /**
   * Determines if the specified character is permissible
   *   in an identifier according to Java.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isIDPart(mixed $cp): mixed;

  /**
   * Determines if the specified character should be regarded as an
   *   ignorable character in an identifier, according to Java.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isIDIgnorable(mixed $cp): mixed;

  /**
   * Determines if the specified character is permissible as the
   *   first character in a Java identifier.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isJavaIDStart(mixed $cp): mixed;

  /**
   * Determines if the specified character is permissible in a Java identifier.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return bool - Whether or not the codepoint has this property
   */
  <<__Native, __IsFoldable>>
  public static function isJavaIDPart(mixed $cp): mixed;

  /**
   * Returns the bidirectional category value for the code point,
   *   which is used in the Unicode bidirectional algorithm
   *   (UAX #9 http://www.unicode.org/reports/tr9/).
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return int - Character directionality, i.e. IntlChar::LEFT_TO_RIGHT
   */
  <<__Native, __IsFoldable>>
  public static function charDirection(mixed $cp): mixed;

  /**
   * Returns the general category value for the code point.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return int - The general category value for the code point.
   */
  <<__Native, __IsFoldable>>
  public static function charType(mixed $cp): mixed;

  /**
   * Returns the combining class of the code point.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return int - The combining class
   */
  <<__Native, __IsFoldable>>
  public static function getCombiningClass(mixed $cp): mixed;

  /**
   * Returns the decimal digit value of a decimal digit character
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return int - Character's digit value
   */
  <<__Native, __IsFoldable>>
  public static function charDigitValue(mixed $cp): mixed;

  /**
   * Maps the specified character to its paired bracket character.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return (int|string) - Paired bracket character
   *   As a UTF-8 sequence if a UTF-8 sequence was provided,
   *   as an integer otherwise.
   *
   * @require ICU >= 52
   */
  <<__Native, __IsFoldable>>
  public static function getBidiPairedBracket(mixed $cp): mixed;

  /**
   * Maps the specified character to a "mirror-image" character.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return (int|string) - The resulting codepoint
   *   As a UTF-8 sequence if a UTF-8 sequence was provided,
   *   as an integer otherwise.
   */
  <<__Native, __IsFoldable>>
  public static function charMirror(mixed $cp): mixed;

  /**
   * The given character is mapped to its lowercase equivalent
   *   according to UnicodeData.txt; if the character has
   *   no lowercase equivalent, the character itself is returned.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return (int|string) - The resulting codepoint
   *   As a UTF-8 sequence if a UTF-8 sequence was provided,
   *   as an integer otherwise.
   */
  <<__Native, __IsFoldable>>
  public static function tolower(mixed $cp): mixed;

  /**
   * The given character is mapped to its uppercase equivalent
   *   according to UnicodeData.txt; if the character has
   *   no uppercase equivalent, the character itself is returned.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return (int|string) - The resulting codepoint
   *   As a UTF-8 sequence if a UTF-8 sequence was provided,
   *   as an integer otherwise.
   */
  <<__Native, __IsFoldable>>
  public static function toupper(mixed $cp): mixed;

  /**
   * The given character is mapped to its titlecase equivalent
   *   according to UnicodeData.txt; if none is defined,
   *   the character itself is returned.
   *
   * @param (int|string) $cp - The codepoint to introspect
   * @return (int|string) - The resulting codepoint
   *   As a UTF-8 sequence if a UTF-8 sequence was provided,
   *   as an integer otherwise.
   */
  <<__Native, __IsFoldable>>
  public static function totitle(mixed $cp): mixed;
}
