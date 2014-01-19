<?php
/**
* A Compatibility library with PHP 5.5's simplified password hashing API.
*
* @author Anthony Ferrara <ircmaxell@php.net>
* @license http://www.opensource.org/licenses/mit-license.html MIT License
* @copyright 2012 The Authors
*
* Some changes by James Miller <james@pocketrent.com> for integration with the
* HipHop VM
*/

/**
* Hash the password using the specified algorithm
*
* @param string $password The password to hash
* @param int $algo The algorithm to use (Defined by PASSWORD_* constants)
* @param array $options The options for the algorithm to use
*
* @return string|false The hashed password, or false on error.
*/
function password_hash(string $password,
                       int $algo,
                       array $options = array()) : mixed {
  if (!function_exists('crypt')) {
    trigger_error("Crypt must be loaded for password_hash to function",
                  E_USER_WARNING);
    return null;
  }
  if (!is_string($password)) {
    trigger_error("password_hash(): Password must be a string",
                  E_USER_WARNING);
    return null;
  }
  if (!is_int($algo)) {
    $algo_type = gettype($algo);
    trigger_error(
      "password_hash() expects parameter 2 to be long, ".$algo_type." given",
      E_USER_WARNING);
    return null;
  }
  switch ($algo) {
    case \PASSWORD_BCRYPT:
      // Note that this is a C constant, but not exposed to PHP, so we don't
      // define it here.
      $cost = 10;
      if (isset($options['cost'])) {
        $cost = $options['cost'];
        if ($cost < 4 || $cost > 31) {
          trigger_error(
            sprintf(
              "password_hash(): Invalid bcrypt cost parameter specified: %d",
              $cost),
            E_USER_WARNING);
          return null;
        }
      }
      // The length of salt to generate
      $raw_salt_len = 16;
      // The length required in the final serialization
      $required_salt_len = 22;
      $hash_format = sprintf("$2y$%02d$", $cost);
      break;
    default:
      trigger_error(
        sprintf(
          "password_hash(): Unknown password hashing algorithm: %s",
          $algo),
        E_USER_WARNING);
      return null;
  }
  if (isset($options['salt'])) {
    switch (gettype($options['salt'])) {
      case 'NULL':
      case 'boolean':
      case 'integer':
      case 'double':
      case 'string':
        $salt = (string) $options['salt'];
        break;
      case 'object':
        if (method_exists($options['salt'], '__tostring')) {
          $salt = (string) $options['salt'];
          break;
        }
      case 'array':
      case 'resource':
      default:
        trigger_error('password_hash(): Non-string salt parameter supplied',
                      E_USER_WARNING);
        return null;
    }
    if (strlen($salt) < $required_salt_len) {
      trigger_error(
        sprintf(
          "password_hash(): Provided salt is too short: %d expecting %d",
          strlen($salt),
          $required_salt_len),
        E_USER_WARNING);
      return null;
    } elseif (0 == preg_match('#^[a-zA-Z0-9./]+$#D', $salt)) {
      $salt = str_replace('+', '.', base64_encode($salt));
    }
  } else {
    $buffer = '';
    $buffer_valid = false;
    if (function_exists('mcrypt_create_iv') && !defined('PHALANGER')) {
      $buffer = mcrypt_create_iv($raw_salt_len, MCRYPT_DEV_URANDOM);
      if ($buffer) {
        $buffer_valid = true;
      }
    }
    if (!$buffer_valid && function_exists('openssl_random_pseudo_bytes')) {
      $buffer = openssl_random_pseudo_bytes($raw_salt_len);
      if ($buffer) {
        $buffer_valid = true;
      }
    }
    if (!$buffer_valid && is_readable('/dev/urandom')) {
      $f = fopen('/dev/urandom', 'r');
      $read = strlen($buffer);
      while ($read < $raw_salt_len) {
        $buffer .= fread($f, $raw_salt_len - $read);
        $read = strlen($buffer);
      }
      fclose($f);
      if ($read >= $raw_salt_len) {
        $buffer_valid = true;
      }
    }
    if (!$buffer_valid || strlen($buffer) < $raw_salt_len) {
      $bl = strlen($buffer);
      for ($i = 0; $i < $raw_salt_len; $i++) {
        if ($i < $bl) {
          $buffer[$i] = $buffer[$i] ^ chr(mt_rand(0, 255));
        } else {
          $buffer .= chr(mt_rand(0, 255));
        }
      }
    }
    $salt = str_replace('+', '.', base64_encode($buffer));
  }
  $salt = substr($salt, 0, $required_salt_len);

  $hash = $hash_format . $salt;

  $ret = crypt($password, $hash);

  if (!is_string($ret) || strlen($ret) <= 13) {
    return false;
  }

  return $ret;
}

/**
* Get information about the password hash. Returns an array of the information
* that was used to generate the password hash.
*
* array(
* 'algo' => 1,
* 'algoName' => 'bcrypt',
* 'options' => array(
* 'cost' => 10,
* ),
* )
*
* @param string $hash The password hash to extract info from
*
* @return array The array of information about the hash.
*/
function password_get_info(string $hash) : array {
  $return = array(
    'algo' => 0,
    'algoName' => 'unknown',
    'options' => array(),
  );
  if (substr($hash, 0, 4) == '$2y$' && strlen($hash) == 60) {
    $return['algo'] = PASSWORD_BCRYPT;
    $return['algoName'] = 'bcrypt';
    list($cost) = sscanf($hash, "$2y$%d$");
    $return['options']['cost'] = $cost;
  }
  return $return;
}

/**
* Determine if the password hash needs to be rehashed according to the options
* provided
*
* If the answer is true, after validating the password using password_verify,
* rehash it.
*
* @param string $hash The hash to test
* @param int $algo The algorithm used for new password hashes
* @param array $options The options array passed to password_hash
*
* @return boolean True if the password needs to be rehashed.
*/
function password_needs_rehash(string $hash,
                               int $algo, array $options = array()) : boolean {
  $info = password_get_info($hash);
  if ($info['algo'] != $algo) {
    return true;
  }
  switch ($algo) {
    case PASSWORD_BCRYPT:
      $cost = isset($options['cost']) ? $options['cost'] : 10;
      if ($cost != $info['options']['cost']) {
        return true;
      }
      break;
  }
  return false;
}

/**
* Verify a password against a hash using a timing attack resistant approach
*
* @param string $password The password to verify
* @param string $hash The hash to verify against
*
* @return boolean If the password matches the hash
*/
function password_verify(string $password, string $hash) : boolean {
  if (!function_exists('crypt')) {
    trigger_error("Crypt must be loaded for password_verify to function",
                  E_USER_WARNING);
    return false;
  }
  $ret = crypt($password, $hash);
  if (!is_string($ret) || strlen($ret) != strlen($hash) || strlen($ret) <= 13) {
    return false;
  }

  $status = 0;
  for ($i = 0; $i < strlen($ret); $i++) {
    $status |= (ord($ret[$i]) ^ ord($hash[$i]));
  }

  return $status === 0;
}
