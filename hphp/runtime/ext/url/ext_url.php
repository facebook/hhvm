<?hh

/**
 * Decodes data encoded with MIME base64
 *
 * @param string $data - The encoded data.
 * @param bool $strict - Return FALSE if input contains character from outside
 *                       the base64 alphabet.
 *
 * @return string - Returns the original data or FALSE on failure. The returned
 *                  data may be binary.
 */
<<__Native>>
function base64_decode(string $data, bool $strict = false): mixed;

/**
 * Encodes data with MIME base64
 *
 * @param string $data - The data to encode.
 *
 * @return string - The encoded data, as a string or FALSE on failure.
 */
<<__Native>>
function base64_encode(string $data): mixed;

/**
 * Fetches all the headers sent by the server in response to a HTTP request
 *
 * @param string $url - The target URL.
 * @param int $format - If the optional $format parameter is set to non-zero,
 *                      get_headers() parses the response and sets the array's
 *                      keys.
 *
 * @return array - Returns an indexed or associative array with the headers, or
 *                 FALSE on failure.
 */
<<__Native>>
function get_headers(string $url, int $format = 0): mixed;

/**
 * Extracts all meta tag content attributes from a file and returns an array
 *
 * @param string $filename       - The path to the HTML file, as a string. This
 *                                 can be a local file or an URL
 * @param bool $use_include_path - Setting use_include_path to TRUE will result
 *                                 in PHP trying to open the file along the
 *                                 standard include path as per the include_path
 *                                 directive. This is used for local files, not
 *                                 URLs.
 *
 * @return array - Returns an array with all the parsed meta tags.
 */
<<__Native>>
function get_meta_tags(string $filename,
                        bool $use_include_path = false): array<string, string>;

/**
 * Generate URL-encoded query string
 *
 * @param mixed $query_data      - May be an array or object containing
 *                                 properties.
 * @param string $numeric_prefix - If numeric indices are used in the base array
 *                                 and this parameter is provided, it will be
 *                                 prepended to the numeric index for elements
 *                                 in the base array only.
 * @param string $arg_separator  - arg_separator.output is used to separate
 *                                 arguments, unless this parameter is
 *                                 specified, and is then used.
 * @param int $nec_type          - By default, PHP_QUERY_RFC1738. If enc_type is
                                   PHP_QUERY_RFC1738, then encoding is performed
                                   per RFC 1738 and the
                                   application/x-www-form-urlencoded media type,
                                   which implies that spaces are encoded as plus
                                   (+) signs. If enc_type is PHP_QUERY_RFC3986,
                                   then encoding is performed according to RFC
                                   3986, and spaces will be percent encoded
                                   (%20).
 *
 * @return string - Returns a URL-encoded string.
 */
<<__Native>>
function http_build_query(
  mixed $query_data,
  string $numeric_prefix = "",
  string $arg_separator = "",
  int $enc_type = PHP_QUERY_RFC1738): mixed;

/**
 * Parse a URL and return its components
 *
 * @param string $url    - The URL to parse.
 * @param int $component - Specify one of the PHP_URL_* constants to retrieve
 *                         just a specific URL component as a string (except
 *                         when PHP_URL_PORT is given, in which case the return
 *                         value will be an integer).
 *
 * @return mixed - On seriously malformed URLs, parse_url() may return FALSE.
 *                 If the component parameter is omitted, an associative array
 *                 is returned. If the component parameter is specified,
 *                 parse_url() returns a string (or an integer, in the case of
 *                 PHP_URL_PORT) instead of an array. If the requested
 *                 component doesn't exist within the given URL, NULL will be
 *                 returned.
 */
<<__Native>>
function parse_url(string $url, int $component = -1): mixed;

/**
 * Decode URL-encoded strings
 *
 * @param string $str - The URL to be decoded.
 *
 * @return string - Returns the decoded URL, as a string.
 */
<<__Native>>
function rawurldecode(string $str): string;

/**
 * URL-encode according to RFC 3986
 *
 * @param string $str - The URL to be encoded.
 *
 * @return string - Returns a string in which all non-alphanumeric characters
 *                  except -_.~ have been replaced with a percent (%) sign
 *                  followed by two hex digits.
 */
<<__Native>>
function rawurlencode(string $str): string;

/**
 * Decode URL-encoded strings
 *
 * @param string $str - The URL to be decoded.
 *
 * @return string - Returns the decoded URL, as a string.
 */
<<__Native>>
function urldecode(string $str): string;

/**
 * URL-encodes string
 *
 * @param string $str - The string to be encoded.
 *
 * @return string - Returns a string in which all non-alphanumeric characters
 *                  except -_. have been replaced with a percent (%) sign
 *                  followed by two hex digits and spaces encoded as plus (+)
 *                  signs.
 */
<<__Native>>
function urlencode(string $str): string;
