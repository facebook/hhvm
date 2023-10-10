<?hh

const float M_PI       = 3.1415926535898;
const float M_PI_2     = 1.5707963267949;
const float M_PI_4     = 0.78539816339745;
const float M_1_PI     = 0.31830988618379;
const float M_2_PI     = 0.63661977236758;
const float M_2_SQRTPI = 1.1283791670955;
const float M_E        = 2.718281828459;
const float M_EULER    = 0.57721566490153;
const float M_LN10     = 2.302585092994;
const float M_LN2      = 0.69314718055995;
const float M_LNPI     = 1.1447298858494;
const float M_LOG10E   = 0.43429448190325;
const float M_LOG2E    = 1.442695040889;
const float M_SQRT1_2  = 0.70710678118655;
const float M_SQRT2    = 1.4142135623731;
const float M_SQRT3    = 1.7320508075689;
const float M_SQRTPI   = 1.7724538509055;

/**
 * @return float - The value of pi as float.
 *
 */
<<__IsFoldable>>
function pi()[]: float {
  return M_PI;
}

/**
 * If the first and only parameter is an array, min() returns the lowest value
 *   in that array. If at least two parameters are provided, min() returns the
 *   smallest of these values.  PHP will evaluate a non-numeric string as 0 if
 *   compared to integer, but still return the string if it's seen as the
 *   numerically lowest value. If multiple arguments evaluate to 0, min() will
 *   return the lowest alphanumerical string value if any strings are given,
 *   else a numeric 0 is returned.
 *
 * @param mixed $value - An array containing the values.
 * @param mixed $second - A second value to compare.
 *
 * @return mixed - min() returns the numerically lowest of the parameter
 *   values.
 *
 */
<<__IsFoldable, __Native>>
function min(mixed $value1, mixed ...$argv)[]: mixed;

/**
 * If the first and only parameter is an array, max() returns the highest
 *   value in that array. If at least two parameters are provided, max() returns
 *   the biggest of these values.  PHP will evaluate a non-numeric string as 0
 *   if compared to integer, but still return the string if it's seen as the
 *   numerically highest value. If multiple arguments evaluate to 0, max() will
 *   return a numeric 0 if given, else the alphabetical highest string value
 *   will be returned.
 *
 * @param mixed $value - An array containing the values.
 * @param mixed $second - A second value to compare.
 *
 * @return mixed - max() returns the numerically highest of the parameter
 *   values. If multiple values can be considered of the same size, the one that
 *   is listed first will be returned.  When max() is given multiple arrays, the
 *   longest array is returned. If all the arrays have the same length, max()
 *   will use lexicographic ordering to find the return value.  When given a
 *   string it will be cast as an integer when comparing.
 *
 */
<<__IsFoldable, __Native>>
function max(mixed $value1, mixed...$argv)[]: mixed;

/**
 * Returns the absolute value of number.
 *
 * @param mixed $number - The numeric value to process
 *
 * @return mixed - The absolute value of number. If the argument number is of
 *   type float, the return type is also float, otherwise it is integer (as
 *   float usually has a bigger value range than integer).
 *
 */
<<__IsFoldable, __Native>>
function abs(mixed $number)[]: mixed;

/**
 * Checks whether val is a legal finite on this platform.
 *
 * @param float $val - The value to check
 *
 * @return bool - TRUE if val is a legal finite number within the allowed
 *   range for a PHP float on this platform, else FALSE.
 *
 */
<<__IsFoldable, __Native>>
function is_finite(float $val)[]: bool;

/**
 * Returns TRUE if val is infinite (positive or negative), like the result of
 *   log(0) or any value too big to fit into a float on this platform.
 *
 * @param float $val - The value to check
 *
 * @return bool - TRUE if val is infinite, else FALSE.
 *
 */
<<__IsFoldable, __Native>>
function is_infinite(float $val)[]: bool;

/**
 * Checks whether val is 'not a number', like the result of acos(1.01).
 *
 * @param float $val - The value to check
 *
 * @return bool - Returns TRUE if val is 'not a number', else FALSE.
 *
 */
<<__IsFoldable, __Native>>
function is_nan(float $val)[]: bool;

/**
 * @param mixed $number - The value to round
 *
 * @return mixed - value rounded up to the next highest integer. The return
 *   value of ceil() is still of type float as the value range of float is
 *   usually bigger than that of integer.
 *
 */
<<__IsFoldable, __Native>>
function ceil(mixed $number)[]: mixed;

/**
 * @param mixed $number - The numeric value to round
 *
 * @return mixed - value rounded to the next lowest integer. The return value
 *   of floor() is still of type float because the value range of float is
 *   usually bigger than that of integer.
 *
 */
<<__IsFoldable, __Native>>
function floor(mixed $number)[]: mixed;

/**
 * Returns the rounded value of val to specified precision (number of digits
 *   after the decimal point). precision can also be negative or zero (default).
 *    PHP doesn't handle strings like "12,300.2" correctly by default. See
 *   converting from strings.
 *
 * @param mixed $val - The value to round
 * @param int $precision - The optional number of decimal digits to round to.
 * @param int $mode - One of the PHP_ROUND_HALF_* constants to determine how
 *   rounding should occur.
 *
 * @return mixed - The rounded value
 *
 */
<<__IsFoldable, __Native>>
function round(mixed $val,
               int $precision = 0,
               int $mode = PHP_ROUND_HALF_UP)[]: mixed;

/**
 * This function converts number from degrees to the radian equivalent.
 *
 * @param float $number - Angular value in degrees
 *
 * @return float - The radian equivalent of number
 *
 */
<<__IsFoldable>>
function deg2rad(float $number)[]: float {
  return $number / 180.0 * M_PI;
}

/**
 * This function converts number from radian to degrees.
 *
 * @param float $number - A radian value
 *
 * @return float - The equivalent of number in degrees
 *
 */
<<__IsFoldable>>
function rad2deg(float $number)[]: float {
  return $number / M_PI * 180.0;
}


/**
 * Returns a string containing a binary representation of the given number
 *   argument.
 *
 * @param int $number
 *
 * @return string - Binary string representation of number
 *
 */
<<__IsFoldable, __Native>>
function decbin(mixed $number)[]: string;

/**
 * Returns a string containing a hexadecimal representation of the given
 *   number argument. The largest number that can be converted is 4294967295 in
 *   decimal resulting to "ffffffff".
 *
 * @param int $number - Decimal value to convert
 *
 * @return string - Hexadecimal string representation of number
 *
 */
<<__IsFoldable, __Native>>
function dechex(mixed $number)[]: string;

/**
 * Returns a string containing an octal representation of the given number
 *   argument. The largest number that can be converted is 4294967295 in decimal
 *   resulting to "37777777777".
 *
 * @param int $number - Decimal value to convert
 *
 * @return string - Octal string representation of number
 *
 */
<<__IsFoldable, __Native>>
function decoct(mixed $number)[]: string;

/**
 * Returns the decimal equivalent of the binary number represented by the
 *   binary_string argument.  bindec() converts a binary number to an integer
 *   or, if needed for size reasons, float.  bindec() interprets all
 *   binary_string values as unsigned integers. This is because bindec() sees
 *   the most significant bit as another order of magnitude rather than as the
 *   sign bit. Warning  The parameter must be a string. Using other data types
 *   will produce unexpected results.
 *
 * @param string $binary_string - The binary string to convert
 *
 * @return mixed - The decimal value of binary_string
 *
 */
<<__IsFoldable, __Native>>
function bindec(mixed $binary_string)[]: mixed;

/**
 * Returns the decimal equivalent of the hexadecimal number represented by the
 *   hex_string argument. hexdec() converts a hexadecimal string to a decimal
 *   number.  hexdec() will ignore any non-hexadecimal characters it encounters.
 *
 * @param string $hex_string - The hexadecimal string to convert
 *
 * @return mixed - The decimal representation of hex_string
 *
 */
<<__IsFoldable, __Native>>
function hexdec(mixed $hex_string)[]: mixed;

/**
 * Returns the decimal equivalent of the octal number represented by the
 *   octal_string argument.
 *
 * @param string $octal_string - The octal string to convert
 *
 * @return mixed - The decimal representation of octal_string
 *
 */
<<__IsFoldable, __Native>>
function octdec(mixed $octal_string)[]: mixed;

/**
 * Returns a string containing number represented in base tobase. The base in
 *   which number is given is specified in frombase. Both frombase and tobase
 *   have to be between 2 and 36, inclusive. Digits in numbers with a base
 *   higher than 10 will be represented with the letters a-z, with a meaning 10,
 *   b meaning 11 and z meaning 35. Warning base_convert() may lose precision on
 *   large numbers due to properties related to the internal "double" or "float"
 *   type used. Please see the Floating point numbers section in the manual for
 *   more specific information and limitations.
 *
 * @param string $number - The number to convert
 * @param int $frombase - The base number is in
 * @param int $tobase - The base to convert number to
 *
 * @return mixed - number converted to base tobase
 *
 */
<<__IsFoldable, __Native>>
function base_convert(mixed $number, int $frombase, int $tobase)[]: mixed;

/**
 * Returns base raised to the power of exp. Warning  In PHP 4.0.6 and earlier
 *   pow() always returned a float, and did not issue warnings.
 *
 * @param mixed $base - The base to use
 * @param mixed $exp - The exponent
 *
 * @return mixed - base raised to the power of exp. If the result can be
 *   represented as integer it will be returned as type integer, else it will be
 *   returned as type float. If the power cannot be computed FALSE will be
 *   returned instead.
 *
 */
<<__IsFoldable, __Native>>
function pow(mixed $base, mixed $exp)[]: mixed;

/**
 * Returns e raised to the power of arg.  'e' is the base of the natural
 *   system of logarithms, or approximately 2.718282.
 *
 * @param float $arg - The argument to process
 *
 * @return float - 'e' raised to the power of arg
 *
 */
<<__IsFoldable, __Native>>
function exp(float $arg)[]: float;

/**
 * expm1() returns the equivalent to 'exp(arg) - 1' computed in a way that is
 *   accurate even if the value of arg is near zero, a case where 'exp (arg) -
 *   1' would be inaccurate due to subtraction of two numbers that are nearly
 *   equal.
 *
 * @param float $arg - The argument to process
 *
 * @return float - 'e' to the power of arg minus one
 *
 */
<<__IsFoldable, __Native>>
function expm1(float $arg)[]: float;

/**
 * Returns the base-10 logarithm of arg.
 *
 * @param float $arg - The argument to process
 *
 * @return float - The base-10 logarithm of arg
 *
 */
<<__IsFoldable, __Native>>
function log10(float $arg)[]: float;

/**
 * log1p() returns log(1 + number) computed in a way that is accurate even
 *   when the value of number is close to zero. log() might only return log(1)
 *   in this case due to lack of precision.
 *
 * @param float $number - The argument to process
 *
 * @return float - log(1 + number)
 *
 */
<<__IsFoldable, __Native>>
function log1p(float $number)[]: float;

/**
 * If the optional base parameter is specified, log() returns logbase arg,
 *   otherwise log() returns the natural logarithm of arg.
 *
 * @param float $arg - The value to calculate the logarithm for
 * @param float $base - The optional logarithmic base to use (defaults to 'e'
 *   and so to the natural logarithm).
 *
 * @return float - The logarithm of arg to base, if given, or the natural
 *   logarithm.
 *
 */
<<__IsFoldable, __Native>>
function log(float $arg, float $base = 0.0)[]: float;

/**
 * cos() returns the cosine of the arg parameter. The arg parameter is in
 *   radians.
 *
 * @param float $arg - An angle in radians
 *
 * @return float - The cosine of arg
 *
 */
<<__IsFoldable, __Native>>
function cos(float $arg)[]: float;

/**
 * Returns the hyperbolic cosine of arg, defined as (exp(arg) + exp(-arg))/2.
 *
 * @param float $arg - The argument to process
 *
 * @return float - The hyperbolic cosine of arg
 *
 */
<<__IsFoldable, __Native>>
function cosh(float $arg)[]: float;

/**
 * sin() returns the sine of the arg parameter. The arg parameter is in
 *   radians.
 *
 * @param float $arg - A value in radians
 *
 * @return float - The sine of arg
 *
 */
<<__IsFoldable, __Native>>
function sin(float $arg)[]: float;

/**
 * Returns the hyperbolic sine of arg, defined as (exp(arg) - exp(-arg))/2.
 *
 * @param float $arg - The argument to process
 *
 * @return float - The hyperbolic sine of arg
 *
 */
<<__IsFoldable, __Native>>
function sinh(float $arg)[]: float;

/**
 * tan() returns the tangent of the arg parameter. The arg parameter is in
 *   radians.
 *
 * @param float $arg - The argument to process in radians
 *
 * @return float - The tangent of arg
 *
 */
<<__IsFoldable, __Native>>
function tan(float $arg)[]: float;

/**
 * Returns the hyperbolic tangent of arg, defined as sinh(arg)/cosh(arg).
 *
 * @param float $arg - The argument to process
 *
 * @return float - The hyperbolic tangent of arg
 *
 */
<<__IsFoldable, __Native>>
function tanh(float $arg)[]: float;

/**
 * Returns the arc cosine of arg in radians. acos() is the complementary
 *   function of cos(), which means that a==cos(acos(a)) for every value of a
 *   that is within acos()' range.
 *
 * @param float $arg - The argument to process
 *
 * @return float - The arc cosine of arg in radians.
 *
 */
<<__IsFoldable, __Native>>
function acos(float $arg)[]: float;

/**
 * Returns the inverse hyperbolic cosine of arg, i.e. the value whose
 *   hyperbolic cosine is arg.
 *
 * @param float $arg - The value to process
 *
 * @return float - The inverse hyperbolic cosine of arg
 *
 */
<<__IsFoldable, __Native>>
function acosh(float $arg)[]: float;

/**
 * Returns the arc sine of arg in radians. asin() is the complementary
 *   function of sin(), which means that a==sin(asin(a)) for every value of a
 *   that is within asin()'s range.
 *
 * @param float $arg - The argument to process
 *
 * @return float - The arc sine of arg in radians
 *
 */
<<__IsFoldable, __Native>>
function asin(float $arg)[]: float;

/**
 * Returns the inverse hyperbolic sine of arg, i.e. the value whose hyperbolic
 *   sine is arg.
 *
 * @param float $arg - The argument to process
 *
 * @return float - The inverse hyperbolic sine of arg
 *
 */
<<__IsFoldable, __Native>>
function asinh(float $arg)[]: float;

/**
 * Returns the arc tangent of arg in radians. atan() is the complementary
 *   function of tan(), which means that a==tan(atan(a)) for every value of a
 *   that is within atan()'s range.
 *
 * @param float $arg - The argument to process
 *
 * @return float - The arc tangent of arg in radians.
 *
 */
<<__IsFoldable, __Native>>
function atan(float $arg)[]: float;

/**
 * Returns the inverse hyperbolic tangent of arg, i.e. the value whose
 *   hyperbolic tangent is arg.
 *
 * @param float $arg - The argument to process
 *
 * @return float - Inverse hyperbolic tangent of arg
 *
 */
<<__IsFoldable, __Native>>
function atanh(float $arg)[]: float;

/**
 * @param float $y - Dividend parameter
 * @param float $x - Divisor parameter
 *
 * @return float - The arc tangent of y/x in radians.
 *
 */
<<__IsFoldable, __Native>>
function atan2(float $y, float $x)[]: float;

/**
 * hypot() returns the length of the hypotenuse of a right-angle triangle with
 *   sides of length x and y, or the distance of the point (x, y) from the
 *   origin. This is equivalent to sqrt(x*x + y*y).
 *
 * @param float $x - Length of first side
 * @param float $y - Length of second side
 *
 * @return float - Calculated length of the hypotenuse
 *
 */
<<__IsFoldable, __Native>>
function hypot(float $x, float $y)[]: float;

/**
 * Returns the floating point remainder of dividing the dividend (x) by the
 *   divisor (y). The reminder (r) is defined as: x = i * y + r, for some
 *   integer i. If y is non-zero, r has the same sign as x and a magnitude less
 *   than the magnitude of y.
 *
 * @param float $x - The dividend
 * @param float $y - The divisor
 *
 * @return float - The floating point remainder of x/y
 *
 */
<<__IsFoldable, __Native>>
function fmod(float $x, float $y)[]: float;

/**
 * Returns the square root of arg.
 *
 * @param float $arg - The argument to process
 *
 * @return float - The square root of arg or the special value NAN for
 *   negative numbers.
 *
 */
<<__IsFoldable, __Native>>
function sqrt(float $arg)[]: float;

/**
 * @return int - The largest possible random value returned by rand()
 *
 */
<<__IsFoldable, __Native>>
function getrandmax()[]: int;

/**
 * Seeds the random number generator with seed or with a random value if no
 *   seed is given. As of PHP 4.2.0, there is no need to seed the random number
 *   generator with srand() or mt_srand() as this is now done automatically.
 *
 * @param mixed $seed - Optional seed value
 *
 */
<<__Native>>
function srand(mixed $seed = null): void;

/**
 * @param int $min - The lowest value to return (default: 0)
 * @param int $max - The highest value to return (default: getrandmax())
 *
 * @return int - A pseudo random value between min (or 0) and max (or
 *   getrandmax(), inclusive).
 *
 */
<<__Native>>
function rand(int $min = 0, ?int $max = null): int;

/**
 * @return int - Returns the maximum random value returned by mt_rand()
 *
 */
<<__IsFoldable, __Native>>
function mt_getrandmax()[]: int;

/**
 * Seeds the random number generator with seed or with a random value if no
 *   seed is given. As of PHP 4.2.0, there is no need to seed the random number
 *   generator with srand() or mt_srand() as this is now done automatically.
 *
 * @param mixed $seed - An optional seed value
 *
 */
<<__Native>>
function mt_srand(mixed $seed = null): void;

/**
 * @param int $min - Optional lowest value to be returned (default: 0)
 * @param int $max - Optional highest value to be returned (default:
 *   mt_getrandmax())
 *
 * @return int - A random integer value between min (or 0) and max (or
 *   mt_getrandmax(), inclusive)
 *
 */
<<__Native>>
function mt_rand(int $min = 0, ?int $max = null)[leak_safe]: int;

/**
 * lcg_value() returns a pseudo random number in the range of (0, 1). The
 *   function combines two CGs with periods of 2^31 - 85 and 2^31 - 249. The
 *   period of this function is equal to the product of both primes.
 *
 * @return float - A pseudo random float value in the range of (0, 1)
 *
 */
<<__Native>>
function lcg_value(): float;

/**
 * intdiv() Integer division.
 *
 * @param int numerator - Number to be divided.
 * @param int divisor - Number which divides the numerator.
 *
 * @return mixed - The integer division of numerator by divisor. If divisor
 *   is 0, a DivisionByZeroError exception is thrown. If the numerator is
 *   PHP_INT_MIN and the divisor is -1, then an ArithmeticError exception
 *   is thrown.
 *
 */
<<__IsFoldable, __Native>>
function intdiv(int $numerator, int $divisor)[]: mixed;
