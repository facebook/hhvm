<?hh

/**
 * hash() - http://php.net/function.hash
 *
 * @param string $algo     - Name of selected hashing algorithm
 *                           (i.e. "md5", "sha256", "haval160,4", etc..)
 * @param string $data     - Message to be hashed.
 * @param bool $raw_output - When set to TRUE, outputs raw binary data.
 *                           FALSE outputs lowercase hexits.
 *
 * @return string - The calculated message digest as lowercase hexits
 *                  unless raw_output is set to true in which case the
 *                  raw binary representation of the message digest is
 *                  returned.
 *                  On error, FALSE is returned.
 */
<<__Native>>
function hash(string $algo, string $data,
              bool $raw_output = false): mixed;

/**
 * hash_algos() - http://php.net/function.hash-algos
 *
 * @return array - A numerically indexed array containing the list of
 *                  supported hashing algorithms.
 */
<<__Native, __IsFoldable>>
function hash_algos()[]: varray<string>;

/**
 * hash_file() - http://php.net/function.hash-file
 *
 * @param string $algo     - Name of selected hashing algorithm
 *                           (i.e. "md5", "sha256", "haval160,4", etc..)
 * @param string $filename - File whose contents are to be hashed.
 * @param bool $raw_output - When set to TRUE, outputs raw binary data.
 *                           FALSE outputs lowercase hexits.
 *
 * @return string - The calculated message digest as lowercase hexits
 *                  unless raw_output is set to true in which case the
 *                  raw binary representation of the message digest is
 *                  returned.
 *                  On error, FALSE is returned.
 */
<<__Native>>
function hash_file(string $algo, string $filename,
                   bool $raw_output = false): mixed;

/**
 * hash_final() - http://php.net/function.hash-final
 *
 * @param resource $context - Hashing context returned by hash_init().
 * @param bool $raw_output - When set to TRUE, outputs raw binary data.
 *                           FALSE outputs lowercase hexits.
 *
 * @return string - Returns a string containing the calculated message
 *                  digest as lowercase hexits unless raw_output is set
 *                  to true in which case the raw binary representation
 *                  of the message digest is returned.
 */
<<__Native>>
function hash_final(resource $context, bool $raw_output = false)[write_props]: mixed;

/**
 * hash() - http://php.net/function.hash
 *
 * @param string $algo     - Name of selected hashing algorithm
 *                           (i.e. "md5", "sha256", "haval160,4", etc..)
 * @param mixed $data      - Message to be hashed. Will be cast to a string
 * @param string $key      - Shared secret key used for generating the
 *                           HMAC variant of the message digest.
 * @param bool $raw_output - When set to TRUE, outputs raw binary data.
 *                           FALSE outputs lowercase hexits.
 *
 * @return string - The calculated message digest as lowercase hexits
 *                  unless raw_output is set to true in which case the
 *                  raw binary representation of the message digest is
 *                  returned.
 *                  On error, FALSE is returned.
 */
function hash_hmac(string $algo,
                   mixed $data,
                   string $key,
                   ?bool $raw_output = false)[write_props]: mixed {
  // hash_init() doesn't allow empty keys (for good reason)
  // but hash_hmac() needs to support them.
  // Rely on the fact that HMAC keys are null padded
  // making a key of "\0" equivalent to ""
  $ctx = hash_init($algo, HASH_HMAC, ($key !== '') ? $key : "\0");
  if (!($ctx is resource)) {
    return false;
  }
  hash_update($ctx, (string) $data);
  return hash_final($ctx, HH\FIXME\UNSAFE_CAST<?bool, bool>($raw_output));
}

/**
 * hash_hmac_file() - http://php.net/function.hash-hmac-file
 *
 * @param string $algo     - Name of selected hashing algorithm
 *                           (i.e. "md5", "sha256", "haval160,4", etc..)
 * @param string $filename - File who's contents are to be hashed.
 * @param string $key      - Shared secret key used for generating the
 *                           HMAC variant of the message digest.
 * @param bool $raw_output - When set to TRUE, outputs raw binary data.
 *                           FALSE outputs lowercase hexits.
 *
 * @return string - The calculated message digest as lowercase hexits
 *                  unless raw_output is set to true in which case the
 *                  raw binary representation of the message digest is
 *                  returned.
 *                  On error, FALSE is returned.
 */
function hash_hmac_file(string $algo,
                        string $filename,
                        string $key,
                        ?bool $raw_output = false): mixed {
  // hash_init() doesn't allow empty keys (for good reason)
  // but hash_hmac_file() needs to support them.
  // Rely on the fact that HMAC keys are null padded
  // making a key of "\0" equivalent to ""
  $ctx = hash_init($algo, HASH_HMAC, ($key !== '') ? $key : "\0");
  if (!($ctx is resource)) {
    return false;
  }
  hash_update_file($ctx, $filename);
  return hash_final($ctx, HH\FIXME\UNSAFE_CAST<?bool, bool>($raw_output));
}

/**
 * hash_init() - http://php.net/function.hash-init
 *
 * @param string $algo - Name of selected hashing algorithm
 *                       (i.e. "md5", "sha256", "haval160,4", etc..)
 * @param int $options - Optional settings for hash generation,
 *                       currently supports only one option:
 *                       HASH_HMAC. When specified, the key
 *                       must be specified.
 * @param string $key  - When HASH_HMAC is specified for options,
 *                       a shared secret key to be used with the HMAC
 *                       hashing method must be supplied in this parameter.
 *
 * @return resrouce - Returns a Hashing Context resource for use with
 *                    hash_update(), hash_update_stream(), hash_update_file(),
 *                    and hash_final().
 *                    Returns FALSE on failure.
 */
<<__Native>>
function hash_init(string $algo, int $options = 0,
                   string $key = "")[]: mixed;

/**
 * hash_update() - http://php.net/function.hash-update
 *
 * @param resource $context - Hashing context returned by hash_init().
 * @param string $data      - Message to be included in the hash digest.
 *
 * @return bool - Returns TRUE on success, FALSE on failure.
 */
<<__Native>>
function hash_update(resource $context, string $data)[write_props]: bool;

/**
 * hash_update_file() - http://php.net/function.hash-update-file
 *
 * @param resource $context - Hashing context returned by hash_init().
 * @param string $filename  - URL describing location of file to be hashed.
 * @param resource $stream_context - fopen() steam context.
 *
 * @return bool - Returns TRUE on success, FALSE on failure
 */
function hash_update_file(mixed $context, string $filename,
                          mixed $stream_context = null): bool {
  $fp = fopen($filename, 'r', false, $stream_context);
  if (!($fp is resource)) {
    return false;
  }
  for (
    $data = HH\FIXME\UNSAFE_CAST<mixed, string>(fread($fp, 1024));
    strlen($data);
    $data = HH\FIXME\UNSAFE_CAST<mixed, string>(fread($fp, 1024))
  ) {
    if (!hash_update(HH\FIXME\UNSAFE_CAST<mixed, resource>($context), $data)) {
      return false;
    }
  }
  fclose($fp);
  return true;
}

/**
 * hash_update_stream - http://php.net/function.hash-update-stream
 *
 * @param resource $context - Hashing context returned by hash_init().
 * @param resource $handle  - Open file handle as returned by any
 *                            stream creation function.
 * @param int $maxlen       - Maximum number of characters to copy
 *                            from handle into the hashing context.
 *
 * @return int - Actual number of bytes added to the hashing context
 *               from handle.
 */
function hash_update_stream(mixed $context, mixed $handle,
                            int $maxlen = -1): int {
  $didread = 0;
  $context = HH\FIXME\UNSAFE_CAST<mixed, resource>($context);
  $handle = HH\FIXME\UNSAFE_CAST<mixed, resource>($handle);
  while ($maxlen) {
    $chunk = HH\FIXME\UNSAFE_CAST<mixed, string>(
      fread($handle, ($maxlen > 0) ? $maxlen : 1024)
    );
    $len = strlen($chunk);
    if (!$len) {
      return $didread;
    }
    if (!hash_update($context, $chunk)) {
      fseek($handle, -$len, SEEK_CUR);
      return $didread;
    }
    $didread += $len;
    $maxlen -= $len;
  }
  return $didread;
}

/**
 * hash_copy - Copy hashing context
 *
 * @param resource $context - Hashing context returned by hash_init().
 *
 * @return resource - Returns a copy of Hashing Context resource.
 */
<<__Native>>
function hash_copy(resource $context): mixed;

/**
 * hash_pbkdf2 - http://php.net/function.hash-pbkdf2.php
 * RFC 2898 - http://www.ietf.org/rfc/rfc2898.txt
 *
 * @param string $algo     - Name of selected hashing algorithm
 *                           (i.e. "md5", "sha256", "haval160,4", etc..)
 * @param string $password - The password to use for the derivation.
 * @param string $salt     - The salt to use for the derivation. This value
 *                           should be generated randomly.
 * @param int $iterations  - The number of internal iterations to perform
 *                           for the derivation.
 * @param int $length      - The length of the output string. If raw_output is
 *                           TRUE this corresponds to the byte-length of the
 *                           derived key, if raw_output is FALSE this
 *                           corresponds to twice the byte-length of the
 *                           derived key (as every byte of the key is returned
 *                           as two hexits).
 *                           If 0 is passed, the entire output of the supplied
 *                           algorithm is used.
 * @param bool $raw_output - When set to TRUE, outputs raw binary data.
 *                           FALSE outputs lowercase hexits.
 *
 * @return string - A string containing the derived key as lowercase hexits
 *                  unless raw_output is set to TRUE in which case the raw
 *                  binary representation of the derived key is returned.
 */
function hash_pbkdf2(string $algo, string $password, string $salt,
                     int $iterations, int $length = 0,
                     bool $raw_output = false): mixed {
  $algo = strtolower($algo);
  if (!in_array($algo, hash_algos())) {
    error_log("\nWarning: hash_pbkdf2(): Unknown hashing algorithm: ".
              $algo);
    return false;
  }

  if ($iterations <= 0) {
    error_log("\nWarning: hash_pbkdf2(): Iterations must be a positive".
              " integer: ".$iterations);
    return false;
  }

  if ($length < 0) {
    error_log("\nWarning: hash_pbkdf2(): Length must be greater than or ".
              "equal to 0: ".$length);
    return false;
  }

  $result = "";
  $hash_length = strlen(
    HH\FIXME\UNSAFE_CAST<mixed, string>(hash($algo, "", true))
  );
  if (!$length) {
    $length = $hash_length;
    if (!$raw_output) {
      // It's a bit weird
      $length *= 2;
    }
  }
  $key_blocks = HH\FIXME\UNSAFE_CAST<mixed, num>(ceil($length / $hash_length));
  for ($i = 1; $i <= $key_blocks; $i++) {
    // Note: $i encoded with most siginificant octet first.
    $xor = HH\FIXME\UNSAFE_CAST<mixed, string>(hash_hmac(
      $algo,
      $salt.HH\FIXME\UNSAFE_CAST<mixed, string>(pack("N", $i)),
      $password,
      true,
    ));
    $prev = $xor;
    for ($j = 1; $j < $iterations; $j++) {
      $prev = HH\FIXME\UNSAFE_CAST<mixed, string>(
        hash_hmac($algo, $prev, $password, true)
      );
      $xor = \HH\str_bitwise_xor($xor, $prev);
    }
    $result .= $xor;
  }

  if ($raw_output) {
    return substr($result, 0, $length);
  } else {
    return substr(bin2hex($result), 0, $length);
  }
  return $result;
}

/**
 * hash_equals - Compare two strings in constant time
 *
 * Note that this function is meant to be slower than a standard
 * strcmp which short-circuits the comparison once the result is
 * known.
 *
 * This function is meant to take the same amount of time
 * (based on user_string length) regardless of the length or
 * contents of known_string.
 *
 * @param string $known - Fixed string to be compared against
 * @param string $user - User-supplied string to check
 *
 * @return bool - Whether $known == $user
 */
// While this function could be marked __IsFoldable, doing so would defeat
// the purpose of having a comparison function which takes a fixed amount
// of time.
<<__Native>>
function hash_equals(mixed $known, mixed $user)[]: bool;

/**
 * furchash_hphp_ext
 *
 * @param string $key - The key to hash
 * @param int $len    - Number of bytes to use from the hash
 * @param int $npart  - The number of buckets
 *
 * @return int - A number in the range of 0-(nPart-1)
 */
<<__Native, __IsFoldable>>
function furchash_hphp_ext(string $key, int $len, int $npart)[]: int;

/**
 * furchash_hphp_ext_supported
 *
 * @return bool - True
 */
<<__IsFoldable>>
function furchash_hphp_ext_supported()[]: bool {
  return true;
}

/**
 * hphp_murmurhash
 *
 * @param string $key - The key to hash
 * @param int $len    - Number of bytes to use from the key
 * @param int $seed   - The seed to use for hashing
 *
 * @return - The Int64 hash of the first len input characters
 */
<<__Native, __IsFoldable>>
function hphp_murmurhash(string $key, int $len, int $seed)[]: int;
