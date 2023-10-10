<?hh

const int ICONV_MIME_DECODE_CONTINUE_ON_ERROR = 2;
const int ICONV_MIME_DECODE_STRICT = 1;

/**
 * Retrieve internal configuration variables of iconv extension
 *
 * @param string $type - The value of the optional type can be:  all
 *   input_encoding output_encoding internal_encoding
 *
 * @return mixed - Returns the current value of the internal
 *   configuration variable if successful.   If type is omitted or set to
 *   "all", iconv_get_encoding() returns an array that stores all these
 *   variables.
 */
<<__Native>>
function iconv_get_encoding(string $type = "all"): mixed;

/**
 * Decodes multiple  header fields at once
 *
 * @param string $encoded_headers - The encoded headers, as a string.
 * @param int $mode - mode determines the behaviour in the event
 *   iconv_mime_decode_headers() encounters a malformed MIME header field.
 *   You can specify any combination of the following bitmasks.  Bitmasks
 *   acceptable to iconv_mime_decode_headers()    Value Constant
 *   Description     1 ICONV_MIME_DECODE_STRICT  If set, the given header
 *   is decoded in full conformance with the standards defined in RFC2047.
 *   This option is disabled by default because there are a lot of broken
 *   mail user agents that don't follow the specification and don't produce
 *   correct MIME headers.    2 ICONV_MIME_DECODE_CONTINUE_ON_ERROR  If
 *   set, iconv_mime_decode_headers() attempts to ignore any grammatical
 *   errors and continue to process a given header.
 * @param string $charset - The optional charset parameter specifies the
 *   character set to represent the result by. If omitted,
 *   iconv.internal_encoding will be used.
 *
 * @return array - Returns an associative array that holds a whole set of
 *   MIME header fields specified by encoded_headers on success, or FALSE
 *   if an error occurs during the decoding.   Each key of the return value
 *   represents an individual field name and the corresponding element
 *   represents a field value. If more than one field of the same name are
 *   present, iconv_mime_decode_headers() automatically incorporates them
 *   into a numerically indexed array in the order of occurrence.
 */
<<__Native>>
function iconv_mime_decode_headers(string $encoded_headers,
                                   int $mode = 0,
                                   ?string $charset = null): mixed;

/**
 * Decodes a  header field
 *
 * @param string $encoded_header - The encoded header, as a string.
 * @param int $mode - mode determines the behaviour in the event
 *   iconv_mime_decode() encounters a malformed MIME header field. You can
 *   specify any combination of the following bitmasks.  Bitmasks
 *   acceptable to iconv_mime_decode()    Value Constant Description     1
 *   ICONV_MIME_DECODE_STRICT  If set, the given header is decoded in full
 *   conformance with the standards defined in RFC2047. This option is
 *   disabled by default because there are a lot of broken mail user agents
 *   that don't follow the specification and don't produce correct MIME
 *   headers.    2 ICONV_MIME_DECODE_CONTINUE_ON_ERROR  If set,
 *   iconv_mime_decode_headers() attempts to ignore any grammatical errors
 *   and continue to process a given header.
 * @param string $charset - The optional charset parameter specifies the
 *   character set to represent the result by. If omitted,
 *   iconv.internal_encoding will be used.
 *
 * @return string - Returns a decoded MIME field on success, or FALSE if
 *   an error occurs during the decoding.
 */
<<__Native>>
function iconv_mime_decode(string $encoded_header,
                           int $mode = 0,
                           ?string $charset = null): mixed;

/**
 * Composes a  header field
 *
 * @param string $field_name - The field name.
 * @param string $field_value - The field value.
 * @param array $preferences - You can control the behaviour of
 *   iconv_mime_encode() by specifying an associative array that contains
 *   configuration items to the optional third parameter preferences. The
 *   items supported by iconv_mime_encode() are listed below. Note that
 *   item names are treated case-sensitive.  Configuration items supported
 *   by iconv_mime_encode()    Item Type Description Default value Example
 *      scheme string  Specifies the method to encode a field value by. The
 *   value of this item may be either "B" or "Q", where "B" stands for
 *   base64 encoding scheme and "Q" stands for quoted-printable encoding
 *   scheme.  B B   input-charset string  Specifies the character set in
 *   which the first parameter field_name and the second parameter
 *   field_value are presented. If not given, iconv_mime_encode() assumes
 *   those parameters are presented to it in the iconv.internal_encoding
 *   ini setting.   iconv.internal_encoding  ISO-8859-1   output-charset
 *   string  Specifies the character set to use to compose the MIME header.
 *     iconv.internal_encoding  UTF-8   line-length integer  Specifies the
 *   maximum length of the header lines. The resulting header is "folded"
 *   to a set of multiple lines in case the resulting header field would be
 *   longer than the value of this parameter, according to RFC2822 -
 *   Internet Message Format. If not given, the length will be limited to
 *   76 characters.  76 996   line-break-chars string  Specifies the
 *   sequence of characters to append to each line as an end-of-line sign
 *   when "folding" is performed on a long header field. If not given, this
 *   defaults to "\r\n" (CR LF). Note that this parameter is always treated
 *   as an ASCII string regardless of the value of input-charset.  \r\n \n
 *
 * @return string - Returns an encoded MIME field on success, or FALSE if
 *   an error occurs during the encoding.
 */
<<__Native>>
function iconv_mime_encode(
  string $field_name,
  string $field_value,
  ?shape(
    ?'scheme' => string,
    ?'input-charset' => string,
    ?'output-charset' => string,
    ?'line-length' => int,
    ?'line-break-chars' => string,
  ) $preferences = null,
): mixed;

/**
 * Set current setting for character encoding conversion
 *
 * @param string $type - The value of type can be any one of these:
 *   input_encoding output_encoding internal_encoding
 * @param string $charset - The character set.
 *
 * @return bool -
 */
<<__Native>>
function iconv_set_encoding(string $type,
                            string $charset): bool;

/**
 * Returns the character count of string
 *
 * @param string $str - The string.
 * @param string $charset - If charset parameter is omitted, str is
 *   assumed to be encoded in iconv.internal_encoding.
 *
 * @return int - Returns the character count of str, as an integer.
 */
<<__Native>>
function iconv_strlen(string $str,
                      ?string $charset = null): mixed;

/**
 * Finds position of first occurrence of a needle within a haystack
 *
 * @param string $haystack - The entire string.
 * @param string $needle - The searched substring.
 * @param int $offset - The optional offset parameter specifies the
 *   position from which the search should be performed.
 * @param string $charset - If charset parameter is omitted, string are
 *   assumed to be encoded in iconv.internal_encoding.
 *
 * @return int - Returns the numeric position of the first occurrence of
 *   needle in haystack.   If needle is not found, iconv_strpos() will
 *   return FALSE.
 */
<<__Native>>
function iconv_strpos(string $haystack,
                      string $needle,
                      int $offset = 0,
                      ?string $charset = null): mixed;

/**
 * Finds the last occurrence of a needle within a haystack
 *
 * @param string $haystack - The entire string.
 * @param string $needle - The searched substring.
 * @param string $charset - If charset parameter is omitted, string are
 *   assumed to be encoded in iconv.internal_encoding.
 *
 * @return int - Returns the numeric position of the last occurrence of
 *   needle in haystack.   If needle is not found, iconv_strrpos() will
 *   return FALSE.
 */
<<__Native>>
function iconv_strrpos(string $haystack,
                       string $needle,
                       ?string $charset = null): mixed;

/**
 * Cut out part of a string
 *
 * @param string $str - The original string.
 * @param int $offset - If offset is non-negative, iconv_substr() cuts
 *   the portion out of str beginning at offset'th character, counting from
 *   zero.   If offset is negative, iconv_substr() cuts out the portion
 *   beginning at the position, offset characters away from the end of str.
 * @param int $length - If length is given and is positive, the return
 *   value will contain at most length characters of the portion that
 *   begins at offset (depending on the length of string).   If negative
 *   length is passed, iconv_substr() cuts the portion out of str from the
 *   offset'th character up to the character that is length characters away
 *   from the end of the string. In case offset is also negative, the start
 *   position is calculated beforehand according to the rule explained
 *   above.
 * @param string $charset - If charset parameter is omitted, string are
 *   assumed to be encoded in iconv.internal_encoding.   Note that offset
 *   and length parameters are always deemed to represent offsets that are
 *   calculated on the basis of the character set determined by charset,
 *   whilst the counterpart substr() always takes these for byte offsets.
 *
 * @return string - Returns the portion of str specified by the offset
 *   and length parameters.   If str is shorter than offset characters
 *   long, FALSE will be returned.
 */
<<__Native>>
function iconv_substr(string $str,
                      int $offset,
                      int $length = PHP_INT_MAX,
                      ?string $charset = null): mixed;

/**
 * Convert string to requested character encoding
 *
 * @param string $in_charset - The input charset.
 * @param string $out_charset - The output charset.   If you append the
 *   string //TRANSLIT to out_charset transliteration is activated. This
 *   means that when a character can't be represented in the target
 *   charset, it can be approximated through one or several similarly
 *   looking characters. If you append the string //IGNORE, characters that
 *   cannot be represented in the target charset are silently discarded.
 *   Otherwise, str is cut from the first illegal character and an E_NOTICE
 *   is generated.
 * @param string $str - The string to be converted.
 *
 * @return string - Returns the converted string.
 */
<<__Native>>
function iconv(string $in_charset,
               string $out_charset,
               string $str): mixed;

/**
 * Convert character encoding as output buffer handler
 *
 * @param string $contents -
 * @param int $status -
 *
 * @return string - See ob_start() for information about this handler
 *   return values.
 */
<<__Native>>
function ob_iconv_handler(string $contents,
                          int $status): string;
