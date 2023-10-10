<?hh

/**
 * Derive a key using the scrypt key derivation function.
 * Scrypt employs salting, iteration, and memory-hardness
 * properties to increase the cost of brute-forcing a secret
 * given the derived key.
 *
 * Scrypt is parameterized in order to control the amount
 * of time a derivation will take.  If unsure what to use
 * for the N, r, and p parameters, passing a zero value will
 * select parameters with an estimated calculation time of
 * .15 seconds.
 *
 * @param password - This is a secret string to be encrypted with scrypt
 * @param salt - A random string to be encrypted with the password
 * @param N - General work factor (log-base-2 valued)
 * @param r - blocksize of the underlying hash
 * @param p - parallelization factor
 *
 * @return mixed - Returns a string of the form
 *         $s$N$r$p$base64(salt)$base64(output), false on failure
 */
<<__Native>>
function scrypt_enc(string $password, string $salt, int $N = 0, int $r = 0,
                    int $p = 0): mixed;
