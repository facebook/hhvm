<?hh

/**
 * Function to extract a sequence of default grapheme clusters from a text
 * buffer, which must be encoded in UTF-8.
 *
 * @param string $haystack - String to search.
 * @param int $size - Maximum number items - based on the $extract_type -
 *   to return.
 * @param int $extract_type - Defines the type of units referred to by
 *   the $size parameter:    GRAPHEME_EXTR_COUNT (default) - $size is the
 *   number of default grapheme clusters to extract. GRAPHEME_EXTR_MAXBYTES
 *   - $size is the maximum number of bytes returned.
 *   GRAPHEME_EXTR_MAXCHARS - $size is the maximum number of UTF-8
 *   characters returned.
 * @param int $start - Starting position in $haystack in bytes - if
 *   given, it must be zero or a positive value that is less than or equal
 *   to the length of $haystack in bytes. If $start does not point to the
 *   first byte of a UTF-8 character, the start position is moved to the
 *   next character boundary.
 * @param int $next - Reference to a value that will be set to the next
 *   starting position. When the call returns, this may point to the first
 *   byte position past the end of the string.
 *
 * @return string - A string starting at offset $start and ending on a
 *   default grapheme cluster boundary that conforms to the $size and
 *   $extract_type specified.
 */
<<__Native>>
function grapheme_extract(string $haystack,
                          int $size,
                          int $extract_type,
                          int $start,
                          <<__OutOnly("KindOfInt64")>>
                          inout ?int $next): mixed;

/**
 * Find position (in grapheme units) of first occurrence of a case-insensitive
 * string
 *
 * @param string $haystack - The string to look in. Must be valid UTF-8.
 * @param string $needle - The string to look for. Must be valid UTF-8.
 * @param int $offset - The optional $offset parameter allows you to
 *   specify where in haystack to start searching as an offset in grapheme
 *   units (not bytes or characters). The position returned is still
 *   relative to the beginning of haystack regardless of the value of
 *   $offset.
 *
 * @return int - Returns the position as an integer. If needle is not
 *   found, grapheme_stripos() will return boolean FALSE.
 */
<<__Native>>
function grapheme_stripos(string $haystack,
                          string $needle,
                          int $offset = 0): mixed;

/**
 * Returns part of haystack string from the first occurrence of
 * case-insensitive needle to the end of haystack.
 *
 * @param string $haystack - The input string. Must be valid UTF-8.
 * @param string $needle - The string to look for. Must be valid UTF-8.
 * @param bool $before_needle - If TRUE, grapheme_strstr() returns the
 *   part of the haystack before the first occurrence of the needle
 *   (excluding needle).
 *
 * @return string - Returns the portion of $haystack, or FALSE if $needle
 *   is not found.
 */
<<__Native>>
function grapheme_stristr(string $haystack,
                          string $needle,
                          bool $before_needle = false): mixed;

/**
 * Get string length in grapheme units
 *
 * @param string $input - The string being measured for length. It must
 *   be a valid UTF-8 string.
 *
 * @return int - The length of the string on success, and 0 if the string
 *   is empty.
 */
<<__Native>>
function grapheme_strlen(string $input): mixed;

/**
 * Find position (in grapheme units) of first occurrence of a string
 *
 * @param string $haystack - The string to look in. Must be valid UTF-8.
 * @param string $needle - The string to look for. Must be valid UTF-8.
 * @param int $offset - The optional $offset parameter allows you to
 *   specify where in $haystack to start searching as an offset in grapheme
 *   units (not bytes or characters). The position returned is still
 *   relative to the beginning of haystack regardless of the value of
 *   $offset.
 *
 * @return int - Returns the position as an integer. If needle is not
 *   found, strpos() will return boolean FALSE.
 */
<<__Native>>
function grapheme_strpos(string $haystack,
                         string $needle,
                         int $offset = 0): mixed;

/**
 * Find position (in grapheme units) of last occurrence of a case-insensitive
 * string
 *
 * @param string $haystack - The string to look in. Must be valid UTF-8.
 * @param string $needle - The string to look for. Must be valid UTF-8.
 * @param int $offset - The optional $offset parameter allows you to
 *   specify where in $haystack to start searching as an offset in grapheme
 *   units (not bytes or characters). The position returned is still
 *   relative to the beginning of haystack regardless of the value of
 *   $offset.
 *
 * @return int - Returns the position as an integer. If needle is not
 *   found, grapheme_strripos() will return boolean FALSE.
 */
<<__Native>>
function grapheme_strripos(string $haystack,
                           string $needle,
                           int $offset = 0): mixed;

/**
 * Find position (in grapheme units) of last occurrence of a string
 *
 * @param string $haystack - The string to look in. Must be valid UTF-8.
 * @param string $needle - The string to look for. Must be valid UTF-8.
 * @param int $offset - The optional $offset parameter allows you to
 *   specify where in $haystack to start searching as an offset in grapheme
 *   units (not bytes or characters). The position returned is still
 *   relative to the beginning of haystack regardless of the value of
 *   $offset.
 *
 * @return int - Returns the position as an integer. If needle is not
 *   found, grapheme_strrpos() will return boolean FALSE.
 */
<<__Native>>
function grapheme_strrpos(string $haystack,
                          string $needle,
                          int $offset = 0): mixed;

/**
 * Returns part of haystack string from the first occurrence of needle to the
 * end of haystack.
 *
 * @param string $haystack - The input string. Must be valid UTF-8.
 * @param string $needle - The string to look for. Must be valid UTF-8.
 * @param bool $before_needle - If TRUE, grapheme_strstr() returns the
 *   part of the haystack before the first occurrence of the needle
 *   (excluding the needle).
 *
 * @return string - Returns the portion of string, or FALSE if needle is
 *   not found.
 */
<<__Native>>
function grapheme_strstr(string $haystack,
                         string $needle,
                         bool $before_needle = false): mixed;

/**
 * Return part of a string
 *
 * @param string $string - The input string. Must be valid UTF-8.
 * @param int $start - Start position in default grapheme units. If
 *   $start is non-negative, the returned string will start at the
 *   $start'th position in $string, counting from zero. If $start is
 *   negative, the returned string will start at the $start'th grapheme
 *   unit from the end of string.
 * @param int $length - Length in grapheme units. If $length is given and
 *   is positive, the string returned will contain at most $length grapheme
 *   units beginning from $start (depending on the length of string). If
 *   $length is given and is negative, then that many grapheme units will
 *   be omitted from the end of string (after the start position has been
 *   calculated when a start is negative). If $start denotes a position
 *   beyond this truncation, FALSE will be returned.
 *
 * @return string - Returns the extracted part of $string.
 */
<<__Native>>
function grapheme_substr(string $string,
                         int $start,
                         mixed $length = null): mixed;
