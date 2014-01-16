<?hh

/**
 * bcscale() - http://php.net/function.bcscale
 *
 * @param int $scale - The scale factor.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function bcscale(int $scale): bool;

/**
 * bcadd() - http://php.net/function.bcadd
 *
 * @param string $left  - The left operand, as a string.
 * @param string $right - The right operand, as a string.
 * @param int $scale    - This optional parameter is used to set the number of
 *                        digits after the decimal place in the result. You can
 *                        also set the global default scale for all functions
 *                        by using bcscale().
 *
 * @return string - The sum of the two operands, as a string.
 */
<<__Native>>
function bcadd(string $left, string $right, int $scale = -1): string;

/**
 * bcsub() - http://php.net/function.bcsub
 *
 * @param string $left  - The left operand, as a string.
 * @param string $right - The right operand, as a string.
 * @param int $scale    - This optional parameter is used to set the number of
 *                        digits after the decimal place in the result. You can
 *                        also set the global default scale for all functions
 *                        by using bcscale().
 *
 * @return string - The result of the subtraction, as a string.
 */
<<__Native>>
function bcsub(string $left, string $right, int $scale = -1): string;

/**
 * bccomp() - http://php.net/function.bccomp
 *
 * @param string $left  - The left operand, as a string.
 * @param string $right - The right operand, as a string.
 * @param int $scale    - This optional parameter is used to set the number of
 *                        digits after the decimal place in the result. You can
 *                        also set the global default scale for all functions
 *                        by using bcscale().
 *
 * @return int - Returns 0 if the two operands are equal, 1 if the left_operand
 *               is larger than the right_operand, -1 otherwise.
 */
<<__Native>>
function bccomp(string $left, string $right, int $scale = -1): int;

/**
 * bcmul() - http://php.net/function.bcmul
 *
 * @param string $left  - The left operand, as a string.
 * @param string $right - The right operand, as a string.
 * @param int $scale    - This optional parameter is used to set the number of
 *                        digits after the decimal place in the result. You can
 *                        also set the global default scale for all functions
 *                        by using bcscale().
 *
 * @return string - Returns the result as a string.
 */
<<__Native>>
function bcmul(string $left, string $right, int $scale = -1): string;

/**
 * bcdiv() - http://php.net/function.bcdiv
 *
 * @param string $left  - The left operand, as a string.
 * @param string $right - The right operand, as a string.
 * @param int $scale    - This optional parameter is used to set the number of
 *                        digits after the decimal place in the result. You can
 *                        also set the global default scale for all functions
 *                        by using bcscale().
 *
 * @return string - Returns the result of the division as a string, or NULL if
 *                  right_operand is 0.
 */
<<__Native>>
function bcdiv(string $left, string $right, int $scale = -1): ?string;

/**
 * bcmod() - http://php.net/function.bcmod
 *
 * @param string $left  - The left operand, as a string.
 * @param string $right - The right operand, as a string.
 *
 * @return string - Returns the modulus as a string, or NULL if modulus is 0.
 */
<<__Native>>
function bcmod(string $left, string $right): ?string;

/**
 * bcpow() - http://php.net/function.bcpow
 *
 * @param string $left  - The left operand, as a string.
 * @param string $right - The right operand, as a string.
 * @param int $scale    - This optional parameter is used to set the number of
 *                        digits after the decimal place in the result. You can
 *                        also set the global default scale for all functions
 *                        by using bcscale().
 *
 * @return string - Returns the result as a string.
 */
<<__Native>>
function bcpow(string $left, string $right, int $scale = -1): string;

/**
 * bcpowmod() - http://php.net/function.bcpowmod
 *
 * @param string $left    - The left operand, as a string.
 * @param string $right   - The right operand, as a string.
 * @param string $modulus - The modulus, as a string
 * @param int $scale      - This optional parameter is used to set the number of
 *                          digits after the decimal place in the result. You
 *                          can also set the global default scale for all
 *                          functions by using bcscale().
 *
 * @return string - Returns the result as a string, or NULL if modulus is 0.
 */
<<__Native>>
function bcpowmod(string $left, string $right, string $modulus,
                  int $scale = -1): ?string;

/**
 * bcsqrt() - http://php.net/function.bcsqrt
 *
 * @param string $operand  - The operand, as a string.
 * @param int $scale       - This optional parameter is used to set the number
 *                           of digits after the decimal place in the result.
 *                           You can also set the global default scale for all
 *                           functions by using bcscale().
 *
 * @return string - Returns the square root as a string, or NULL if operand is
 *                  negative.
 */
<<__Native>>
function bcsqrt(string $operand, int $scale = -1): ?string;
