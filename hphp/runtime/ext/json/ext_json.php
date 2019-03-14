<?hh // partial

/**
 * Objects implementing JsonSerializable can customize their JSON
 * representation when encoded with json_encode().
 */
interface JsonSerializable {
  /**
   * Specify data which should be serialized to JSON
   *
   * @return mixed - Returns data which can be serialized by
   *   json_encode(), which is a value of any type other than a resource.
   */
  public function jsonSerialize();

}

/**
 * Decodes a JSON string
 *
 * @param string $json - The json string being decoded.   This function
 *   only works with UTF-8 encoded strings.
 * @param bool $assoc - When TRUE, returned objects will be converted
 *   into associative arrays.
 * @param int $depth - User specified recursion depth.
 * @param int $options - Bitmask of JSON decode options. Currently only
 *   JSON_BIGINT_AS_STRING is supported (default is to cast large integers
 *   as floats)
 *
 * @return mixed - Returns the value encoded in json in appropriate PHP
 *   type. Values true, false and null (case-insensitive) are returned as
 *   TRUE, FALSE and NULL respectively. NULL is returned if the json cannot
 *   be decoded or if the encoded data is deeper than the recursion limit.
 */
<<__Native>>
function json_decode(string $json,
                     bool $assoc = false,
                     int $depth = 512,
                     int $options = 0): mixed;

/**
 * Returns the JSON representation of a value
 *
 * @param mixed $value - The value being encoded. Can be any type except
 *   a resource.   All string data must be UTF-8 encoded.
 * @param int $options - Bitmask consisting of JSON_HEX_QUOT,
 *   JSON_HEX_TAG, JSON_HEX_AMP, JSON_HEX_APOS, JSON_NUMERIC_CHECK,
 *   JSON_PRETTY_PRINT, JSON_UNESCAPED_SLASHES, JSON_FORCE_OBJECT,
 *   JSON_UNESCAPED_UNICODE. The behaviour of these constants is described
 *   on the JSON constants page.
 * @param int $depth - Set the maximum depth. Must be greater than zero.
 *
 * @return mixed - Returns a JSON encoded string on success .
 */
<<__Native>>
function json_encode(mixed $value,
                     int $options = 0,
                     int $depth = 512): mixed;

/**
 * Returns the error string of the last json_encode() or json_decode() call
 *
 * @return string - Returns the error message on success or NULL with
 *   wrong parameters.
 */
<<__Native>>
function json_last_error_msg(): string;

/**
 * Returns the last error occurred
 *
 * @return int - Returns an integer, the value can be one of the
 *   following constants:   JSON error codes    Constant Meaning
 *   Availability     JSON_ERROR_NONE No error has occurred
 *   JSON_ERROR_DEPTH The maximum stack depth has been exceeded
 *   JSON_ERROR_STATE_MISMATCH Invalid or malformed JSON
 *   JSON_ERROR_CTRL_CHAR Control character error, possibly incorrectly
 *   encoded    JSON_ERROR_SYNTAX Syntax error    JSON_ERROR_UTF8 Malformed
 *   UTF-8 characters, possibly incorrectly encoded PHP 5.3.3
 *   JSON_ERROR_RECURSION One or more recursive references in the value to
 *   be encoded PHP 5.5.0   JSON_ERROR_INF_OR_NAN  One or more NAN or INF
 *   values in the value to be encoded  PHP 5.5.0
 *   JSON_ERROR_UNSUPPORTED_TYPE A value of a type that cannot be encoded
 *   was given PHP 5.5.0
 */
<<__Native>>
function json_last_error(): int;
