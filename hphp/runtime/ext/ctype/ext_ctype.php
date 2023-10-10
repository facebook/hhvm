<?hh

/**
 * Check for alphanumeric character(s)
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in text is either a
 *   letter or a digit, FALSE otherwise.
 */
<<__Native>>
function ctype_alnum(mixed $text)[]: bool;

/**
 * Check for alphabetic character(s)
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in text is a letter
 *   from the current locale, FALSE otherwise.
 */
<<__Native>>
function ctype_alpha(mixed $text)[]: bool;

/**
 * Check for control character(s)
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in text is a control
 *   character from the current locale, FALSE otherwise.
 */
<<__Native>>
function ctype_cntrl(mixed $text)[]: bool;

/**
 * Check for numeric character(s)
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in the string text is a
 *   decimal digit, FALSE otherwise.
 */
<<__Native>>
function ctype_digit(mixed $text)[]: bool;

/**
 * Check for any printable character(s) except space
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in text is printable
 *   and actually creates visible output (no white space), FALSE otherwise.
 */
<<__Native>>
function ctype_graph(mixed $text)[]: bool;

/**
 * Check for lowercase character(s)
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in text is a lowercase
 *   letter in the current locale.
 */
<<__Native>>
function ctype_lower(mixed $text)[]: bool;

/**
 * Check for printable character(s)
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in text will actually
 *   create output (including blanks). Returns FALSE if text contains
 *   control characters or characters that do not have any output or
 *   control function at all.
 */
<<__Native>>
function ctype_print(mixed $text)[]: bool;

/**
 * Check for any printable character which is not whitespace or an
 *    alphanumeric character
 *
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in text is printable,
 *   but neither letter, digit or blank, FALSE otherwise.
 */
<<__Native>>
function ctype_punct(mixed $text)[]: bool;

/**
 * Check for whitespace character(s)
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in text creates some
 *   sort of white space, FALSE otherwise. Besides the blank character this
 *   also includes tab, vertical tab, line feed, carriage return and form
 *   feed characters.
 */
<<__Native>>
function ctype_space(mixed $text)[]: bool;

/**
 * Check for uppercase character(s)
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in text is an uppercase
 *   letter in the current locale.
 */
<<__Native>>
function ctype_upper(mixed $text)[]: bool;

/**
 * Check for character(s) representing a hexadecimal digit
 *
 *
 * @param string $text - The tested string.
 *
 * @return bool - Returns TRUE if every character in text is a
 *   hexadecimal 'digit', that is a decimal digit or a character from
 *   [A-Fa-f] , FALSE otherwise.
 */
<<__Native>>
function ctype_xdigit(mixed $text)[]: bool;
