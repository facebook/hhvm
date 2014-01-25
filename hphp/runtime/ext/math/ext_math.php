<?hh

/**
 * Get value of pi
 *
 * @return float - The value of pi as a float
 */
function pi(): float {
  return M_PI;
}

/**
 * Find lowest value
 *
 * @param mixed $values - An array containing the values or list each value
 *                        individually
 *
 * @return mixed - The numerically lowest of the parameter values.
 */
function min(mixed $values, ...): mixed {
  $ret = null;
  if(!is_array($values)) {
    $values = func_get_args();
  }
  if($values) {
    $ret = array_shift($values);
    foreach($values as $value) {
      if($value < $ret) {
        $ret = $value;
      }
    }
  }
  return $ret;
}

/**
 * Find highest value
 *
 * @param mixed $values - An array containing the values or list each value
 *                        individually
 *
 * @return mixed - The numerically highest of the parameter values.
 */
function max(mixed $values, ...): mixed {
  $ret = null;
  if(!is_array($values)) {
    $values = func_get_args();
  }
  if($values) {
    $ret = array_shift($values);
    foreach($values as $value) {
      if($value > $ret) {
        $ret = $value;
      }
    }
  }
  return $ret;
}

/**
 * Absolute value
 *
 * @param mixed $number - The numeric value to process
 * 
 * @return mixed - The absolute value of $number. If the argument $number is of
 *                 type float, the return type is also float, otherwise it is
 *                 integer
 */
<<__Native>>
function abs(mixed $number): mixed;
/**
 * Finds whether a value is a legal finite number
 *
 * @param float $value - The value to check
 *
 * @return bool - TRUE if val is a legal finite number within the allowed range for a
 *                PHP float on this platform, else FALSE.
 */
<<__Native>>
function is_finite(float $value): bool;

/**
 * Finds whether a value is infinite
 *
 * @param float $value - The value to check
 *
 * @return bool - TRUE if val is infinite, else FALSE.
 */
<<__Native>>
function is_infinite(float $value): bool;

/**
 * Finds whether a value is not a number
 *
 * @param float $value - The value to check
 *
 * @return bool - Returns TRUE if val is 'not a number', else FALSE.
 */
<<__Native>>
function is_nan(float $value): bool;

/**
 * Round fractions up
 * 
 * @param float $value - The value to round
 *
 * @return float - $value rounded up to the next highest integer.
 */
<<__Native>>
function ceil(float $value): float;

/**
 * Round fractions down
 * 
 * @param float $value - The value to round
 *
 * @return float - $value rounded to the next lowest integer.
 */
<<__Native>>
function floor(float $value): float;

/**
 * Rounds a float
 * 
 * @param float $value   - The value to round
 * @param int $precision - The optional number of decimal digits to round to.
 * @param int $mode      - Use one of the PHP_ROUND_* constants to specify the
 *                         mode in which rounding occurs.
 *
 * @return float - The rounded value
 */
<<__Native>>
function round(float $value, int $precision = 0,
               int $mode = PHP_ROUND_HALF_UP): float;

/**
 * Converts the number in degrees to the radian equivalent
 * 
 * @param float $number - Angular value in degrees
 * 
 * @return float - The radian equivalent of $number
 */
function deg2rad(float $number): float {
  return $number / 180.0 * M_PI;
}

/**
 * Converts the number in radians to the degrees equivalent
 * 
 * @param float $number - Angular value in radians
 * 
 * @return float - The degrees equivalent of $number
 */
function rad2deg(float $number): float {
  return $number / M_PI * 180.0;
}

/**
 * Decimal to binary
 * 
 * @param int $number - Decimal value to convert
 *
 * @return string - Binary string representation of $number
 */
<<__Native>>
function decbin(int $number): string;

/**
 * Decimal to hexadecimal
 * 
 * @param int $number - Decimal value to convert
 *
 * @return string - Hexadecimal string representation of $number
 */
<<__Native>>
function dechex(int $number): string;

/**
 * Decimal to octal
 * 
 * @param int $number - Decimal value to convert
 *
 * @return string - Octal string representation of $number
 */
<<__Native>>
function decoct(int $number): string;

/**
 * Binary to decimal
 *
 * @param string $binary_string - The binary string to convert
 *
 * @return mixed - The decimal value of $binary_string
 */
<<__Native>>
function bindec(mixed $binary_string): mixed;

/**
 * Hexadecimal to decimal
 *
 * @param string $hex_string - The hexadecimal string to convert
 *
 * @return mixed - The decimal value of $hex_string
 */
<<__Native>>
function hexdec(mixed $hex_string): mixed;

/**
 * Octal to decimal
 *
 * @param string $octal_string - The octal string to convert
 *
 * @return mixed - The decimal value of $octal_string
 */
<<__Native>>
function octdec(mixed $octal_string): mixed;

/**
 * Convert a number between arbitrary bases
 *
 * @param string $number - The number to convert
 * @param int $frombase  - The base $number is in
 * @param int $tobase    - The base to convert $number to
 *
 * @return string - $number converted to base $tobase
 */
<<__Native>>
function base_convert(mixed $number, int $frombase, int $tobase): string;

/**
 * Exponential expression
 *
 * @param mixed $base - The base to use
 * @param mixed $exp  - The exponent
 *
 * @return mixed - $base raised to the power of $exp. If both arguments are
 *                 non-negative integers and the result can be represented as
 *                 an integer, the result will be returned with integer type,
 *                 otherwise it will be returned as a float.
 */
<<__Native>>
function pow(mixed $base, mixed $exp): mixed;

/**
 * Calculates the exponent of e
 *
 * @param float $arg - The argument to process
 *
 * @return float - e raised to the power $arg
 */
<<__Native>>
function exp(float $arg): float;

/**
 * Returns exp(number) - 1, computed in a way that is accurate even when the
 * value of number is close to zero.
 *
 * @param float $arg - The argument to process
 *
 * @return float - e to the power of $arg minus one.
 */
<<__Native>>
function expm1(float $arg): float;

/**
 * Base-10 logarithm
 *
 * @param float $arg - The argument to process
 *
 * @return float - The base-10 logarithm of $arg
 */
<<__Native>>
function log10(float $arg): float;

/**
 * Returns log(1 + number), computed in a way that is accurate even when the
 * value of number is close to zero.
 *
 * @param float $number - The argument to process
 *
 * @return float - log(1 + $number)
 */
<<__Native>>
function log1p(float $number): float;

/**
 * Natural logarithm
 *
 * @param float $arg  - The value to calculate the logarithm for
 * @param float $base - The optional logarithmic base to use
 *
 * @return float - The logarithm of $arg to $base, if given, or the natural
 *                 logarithm.
 */
<<__Native>>
function log(float $number, float $base = 0.0): float;

/**
 * cos
 *
 * @param float $arg - The argument to process
 *
 * @return float - The cos of $arg
 */
<<__Native>>
function cos(float $arg): float;

/**
 * cosh
 *
 * @param float $arg - The argument to process
 *
 * @return float - The cosh of $arg
 */
<<__Native>>
function cosh(float $arg): float;

/**
 * sin
 *
 * @param float $arg - The argument to process
 *
 * @return float - The sin of $arg
 */
<<__Native>>
function sin(float $arg): float;

/**
 * sinh
 *
 * @param float $arg - The argument to process
 *
 * @return float - The sinh of $arg
 */
<<__Native>>
function sinh(float $arg): float;

/**
 * tan
 *
 * @param float $arg - The argument to process
 *
 * @return float - The tan of $arg
 */
<<__Native>>
function tan(float $arg): float;

/**
 * tanh
 *
 * @param float $arg - The argument to process
 *
 * @return float - The tanh of $arg
 */
<<__Native>>
function tanh(float $arg): float;

/**
 * acos
 *
 * @param float $arg - The argument to process
 *
 * @return float - The acos of $arg
 */
<<__Native>>
function acos(float $arg): float;

/**
 * acosh
 *
 * @param float $arg - The argument to process
 *
 * @return float - The acosh of $arg
 */
<<__Native>>
function acosh(float $arg): float;

/**
 * asin
 *
 * @param float $arg - The argument to process
 *
 * @return float - The asin of $arg
 */
<<__Native>>
function asin(float $arg): float;

/**
 * asinh
 *
 * @param float $arg - The argument to process
 *
 * @return float - The asinh of $arg
 */
<<__Native>>
function asinh(float $arg): float;

/**
 * atan
 *
 * @param float $arg - The argument to process
 *
 * @return float - The atan of $arg
 */
<<__Native>>
function atan(float $arg): float;

/**
 * atanh
 *
 * @param float $arg - The argument to process
 *
 * @return float - The atanh of $arg
 */
<<__Native>>
function atanh(float $arg): float;

/**
 * atanh2
 *
 * @param float $y - Dividend parameter
 * @param float $x - Divisor parameter
 *
 * @return float - The arc tangent of $y/$x 
 */
<<__Native>>
function atan2(float $y, float $x): float;

/**
 * Calculate the length of the hypotenuse of a right-angle triangle
 *
 * @param float $x - Length of first side
 * @param float $y - Length of second side
 *
 * @return float - Calculated length of the hypotenuse
 */
function hypot(float $x, float $y): float {
  return sqrt($x * $x + $y * $y);
}

/**
 * Returns the floating point remainder of the division of the arguments
 *
 * @param float $x - The dividend
 * @param float $y - The divisor
 *
 * @return float - The floating point remainder of $x/$y
 */
<<__Native>>
function fmod(float $x, float $y): float;

/**
 * Square root
 *
 * @param float $arg - The argument to process
 *
 * @return The square root of $arg or NAN for negative numbers.
 */
<<__Native>>
function sqrt(float $arg): float;

/**
 * Show largest possible random value
 * 
 * @return int - The largest possible random value returned by rand()
 */
<<__Native>>
function getrandmax(): int;

/**
 * Seed the random number generator
 *
 * @param int $seed - Optional seed value
 */
<<__Native>>
function srand(?int $seed = null): void;

/**
 * Generate a random integer
 *
 * @param int $min - The lowest value to return
 * @param int $max - The highest value to return
 *
 * @return int - A pseudo random value between $min and $max
 */
<<__Native>>
function rand(int $min = 0, int $max = -1): int;

/**
 * Show largest possible random value
 * 
 * @return int - The largest possible random value returned by mt_rand()
 */
<<__Native>>
function mt_getrandmax(): int;

/**
 * Seed the better random number generator
 *
 * @param int $seed - Optional seed value
 */
<<__Native>>
function mt_srand(?int $seed = null): void;

/**
 * Generate a better random value
 *
 * @param int $min - The lowest value to return
 * @param int $max - The highest value to return
 *
 * @return int - A pseudo random value between $min and $max
 */
<<__Native>>
function mt_rand(int $min = 0, int $max = -1): int;

/**
 * Combined linear congruential generator
 *
 * @return float - A pseudo random float value in the range of (0, 1)
 */
<<__Native>>
function lcg_value(): float;
