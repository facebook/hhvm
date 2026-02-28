<?hh

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
  public function jsonSerialize(): mixed;

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
                     int $options = 0)[]: mixed;

/**
 * json_decode, but populates $error in case of error.
 *
 * If the function runs normally with no errors, then $error is set to null.
 * Otherwise, if an error occurs, $error is set to a tuple of (error code
 * constant, description) from the list defined in builtins_json.hhi.
 * */
<<__Native>>
function json_decode_with_error(
  string $json,
  inout ?(int, string) $error,
  bool $assoc = false,
  int $depth = 512,
  int $options = 0,
)[]: mixed;

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
                     int $depth = 512)[defaults]: mixed;


/**
 * json_encode, but populates $error in case of error.
 *
 * If the function runs normally with no errors, then $error is set to null.
 * Otherwise, if an error occurs, $error is set to a tuple of (error code
 * constant, description) from the list defined in builtins_json.hhi.
 * */
<<__Native>>
function json_encode_with_error(
  mixed $value,
  inout ?(int, string) $error,
  int $options = 0,
  int $depth = 512,
)[defaults]: mixed;

/**
 * Like json_encode_with_error but has pure coeffects.
 * Encoding objects implementing JsonSerializable with an impure jsonSerialize
 * will result in coeffect violations.
 */
<<__Native>>
function json_encode_pure(
  mixed $value,
  inout ?(int, string) $error,
  int $options = 0,
  int $depth = 512,
)[]: mixed;
