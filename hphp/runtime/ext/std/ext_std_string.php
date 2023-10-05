<?hh // partial

/* Wraps a string to a given number of characters.
 * @param string $str - The input string.
 * @param int $width - The number of characters at which the string will be
 * wrapped.
 * @param string $break - The line is broken using the optional break
 * parameter.
 * @param bool cut - If the cut is set to true, the string is always wrapped at
 * or before the specified width. So if you have a word that is larger than the
 * given width, it is broken apart.
 * @return mixed - Returns the given string wrapped at the specified length.
 */
<<__Native>>
function wordwrap(string $str, int $width = 75, string $break = "\n",
                  bool $cut = false): mixed;

/* Output a formatted string
 *
 * @param mixed $format - The format string is composed of
 * zero or more directives: ordinary characters (excluding %)
 * that are copied directly to the result, and conversion specifications,
 * each of which results in fetching its own parameter.
 * This applies to both sprintf() and printf().
 *
 * Each conversion specification consists of a percent sign (%),
 * followed by one or more of these elements, in order:
 *
 * An optional sign specifier that forces a sign (- or +) to
 * be used on a number. By default, only the - sign is used
 * on a number if it's negative. This specifier forces positive
 * numbers to have the + sign attached as well,
 * and was added in PHP 4.3.0.
 *
 * An optional padding specifier that says what character will
 * be used for padding the results to the right string size.
 * This may be a space character or a 0 (zero character).
 * The default is to pad with spaces. An alternate padding character
 * can be specified by prefixing it with a single quote (').
 * See the examples below.
 *
 * An optional alignment specifier that says if the result should
 * be left-justified or right-justified. The default is right-justified;
 * a - character here will make it left-justified.
 *
 * An optional number, a width specifier that says how many characters
 * (minimum) this conversion should result in.
 *
 * An optional precision specifier in the form of a period (`.')
 * followed by an optional decimal digit string that says how many
 * decimal digits should be displayed for floating-point numbers.
 * When using this specifier on a string, it acts as a cutoff point,
 * setting a maximum character limit to the string.
 *
 * A type specifier that says what type the argument data should be
 * treated as. Possible types:
 *
 * % - a literal percent character. No argument is required.
 * b - the argument is treated as an integer, and presented as a binary number.
 * c - the argument is treated as an integer, and presented as the
 *     character with that ASCII value.
 * d - the argument is treated as an integer, and presented as a
 *     (signed) decimal number.
 * e - the argument is treated as scientific notation (e.g. 1.2e+2).
 *     The precision specifier stands for the number of digits after
 *     the decimal point since PHP 5.2.1. In earlier versions, it
 *     was taken as number of significant digits (one less).
 * E - like %e but uses uppercase letter (e.g. 1.2E+2).
 * f - the argument is treated as a float, and presented as
 *     a floating-point number (locale aware).
 * F - the argument is treated as a float, and presented as
 *     a floating-point number (non-locale aware).
 *     Available since PHP 4.3.10 and PHP 5.0.3.
 * g - shorter of %e and %f.
 * G - shorter of %E and %f.
 * o - the argument is treated as an integer, and presented as an octal number.
 * s - the argument is treated as and presented as a string.
 * u - the argument is treated as an integer, and presented as
 *     an unsigned decimal number.
 * x - the argument is treated as an integer and presented as a
 *      hexadecimal number (with lowercase letters).
 * X - the argument is treated as an integer and presented as a
 *      hexadecimal number (with uppercase letters).
 *
 * Variables will be co-erced to a suitable type for the specifier:
 */
<<__Native, __IsFoldable>>
function sprintf(mixed $format, mixed... $args)[]: mixed;

/* Return a formatted string
 *
 * Operates as sprintf() but accepts an array of arguments, rather
 * than a variable number of arguments.
 *
 * @param string $format - See sprintf() for a description of format.
 */
<<__Native, __IsFoldable>>
function vsprintf(mixed $format, mixed $args)[]: mixed;

/**
 * Produces output according to format.
 *
 * @param string $format - See sprintf() for a description of format.
 */
<<__Native("NoRecording")>>
function printf(mixed $format, mixed... $args): mixed;

/* Output a formatted string
 *
 * Display array values as a formatted string according to format
 * (which is described in the documentation for sprintf()).
 *
 * Operates as printf() but accepts an array of arguments,
 * rather than a variable number of arguments.
 */
<<__Native("NoRecording")>>
function vprintf(mixed $format, mixed $args): mixed;
