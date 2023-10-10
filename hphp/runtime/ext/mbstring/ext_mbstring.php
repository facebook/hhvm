<?hh

const int MB_OVERLOAD_MAIL = 1;
const int MB_OVERLOAD_STRING = 2;
const int MB_OVERLOAD_REGEX = 4;

/**
 * Returns an array containing all supported encodings.
 *
 * @return array - Returns a numerically indexed array.
 *
 */
<<__Native>>
function mb_list_encodings()[]: varray<string>;

/**
 * @param string $name
 *
 * @return mixed
 *
 */
<<__Native>>
function mb_list_encodings_alias_names(?string $name = null)[]: mixed;

/**
 * @param string $name
 *
 * @return mixed
 *
 */
<<__Native>>
function mb_list_mime_names(?string $name = null)[]: mixed;

/**
 * Checks if the specified byte stream is valid for the specified encoding. It
 *   is useful to prevent so-called "Invalid Encoding Attack".
 *
 * @param string $var - The byte stream to check. If it is omitted, this
 *   function checks all the input from the beginning of the request.
 * @param string $encoding - The expected encoding.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function mb_check_encoding(?string $var = null, ?string $encoding = null)[read_globals]: bool;

/**
 * Performs case folding on a string, converted in the way specified by mode.
 *
 * @param string $str - The string being converted.
 * @param int $mode - The mode of the conversion. It can be one of
 *   MB_CASE_UPPER, MB_CASE_LOWER, or MB_CASE_TITLE.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - A case folded version of string converted in the way
 *   specified by mode.
 *
 */
<<__Native>>
function mb_convert_case(string $str, int $mode,
                         ?string $encoding = null)[read_globals]: mixed;

/**
 * Converts the character encoding of string str to to_encoding from
 *   optionally from_encoding.
 *
 * @param string $str - The string being encoded.
 * @param string $to_encoding - The type of encoding that str is being
 *   converted to.
 * @param mixed $from_encoding - Is specified by character code names before
 *   conversion. It is either an array, or a comma separated enumerated list. If
 *   from_encoding is not specified, the internal encoding will be used. "auto"
 *   may be used, which expands to "ASCII,JIS,UTF-8,EUC-JP,SJIS".
 *
 * @return mixed - The encoded string.
 *
 */
<<__Native>>
function mb_convert_encoding(string $str,
                             string $to_encoding,
                             mixed $from_encoding = null)[read_globals]: mixed;

/**
 * Performs a "han-kaku" - "zen-kaku" conversion for string str. This function
 *   is only useful for Japanese.
 *
 * @param string $str - The string being converted.
 * @param string $option - The conversion option. Specify with a combination
 *   of following options.
 *   Applicable Conversion Options
 *   Option Meaning
 *   r      Convert "zen-kaku" alphabets to "han-kaku"
 *   R      Convert "han-kaku" alphabets to "zen-kaku"
 *   n      Convert "zen-kaku" numbers to "han-kaku"
 *   N      Convert "han-kaku" numbers to "zen-kaku"
 *   a      Convert "zen-kaku" alphabets and numbers to "han-kaku"
 *   A      Convert "han-kaku" alphabets and numbers to "zen-kaku" (Characters
 *          included in "a", "A" options are U+0021 - U+007E excluding U+0022,
 *          U+0027, U+005C, U+007E)
 *   s      Convert "zen-kaku" space to "han-kaku" (U+3000 -> U+0020)
 *   S      Convert "han-kaku" space to "zen-kaku" (U+0020 -> U+3000)
 *   k      Convert "zen-kaku kata-kana" to "han-kaku kata-kana"
 *   K      Convert "han-kaku kata-kana" to "zen-kaku kata-kana"
 *   h      Convert "zen-kaku hira-gana" to "han-kaku kata-kana"
 *   H      Convert "han-kaku kata-kana" to "zen-kaku hira-gana"
 *   c      Convert "zen-kaku kata-kana" to "zen-kaku hira-gana"
 *   C      Convert "zen-kaku hira-gana" to "zen-kaku kata-kana"
 *   V      Collapse voiced sound notation and convert them into a character.
 *          Use with "K","H"
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - The converted string.
 *
 */
<<__Native>>
function mb_convert_kana(string $str,
                         ?string $option = null,
                         ?string $encoding = null)[read_globals]: mixed;

/**
 * Converts character encoding of variables vars in encoding from_encoding to
 *   encoding to_encoding. mb_convert_variables() join strings in Array or
 *   Object to detect encoding, since encoding detection tends to fail for short
 *   strings. Therefore, it is impossible to mix encoding in single array or
 *   object.
 *
 * @param string $to_encoding - The encoding that the string is being
 *   converted to.
 * @param mixed $from_encoding - from_encoding is specified as an array or
 *   comma separated string, it tries to detect encoding from from-coding. When
 *   from_encoding is omitted, detect_order is used.
 * @param mixed $vars - vars is the reference to the variable being converted.
 *   String, Array and Object are accepted. mb_convert_variables() assumes all
 *   parameters have the same encoding.
 *
 * @return mixed - The character encoding before conversion for success, or
 *   FALSE for failure.
 *
 */
<<__Native>>
function mb_convert_variables(string $to_encoding,
                              mixed $from_encoding,
                              inout mixed $vars,
                              mixed... $argv)[read_globals]: mixed;

/**
 * Decodes encoded-word string str in MIME header.
 *
 * @param string $str - The string being decoded.
 *
 * @return mixed - The decoded string in internal character encoding.
 *
 */
<<__Native>>
function mb_decode_mimeheader(string $str)[read_globals]: mixed;

/**
 * Convert numeric string reference of string str in a specified block to
 *   character.
 *
 * @param string $str - The string being decoded.
 * @param mixed $convmap - convmap is an array that specifies the code area to
 *   convert.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - The converted string.
 *
 */
<<__Native>>
function mb_decode_numericentity(string $str,
                                 mixed $convmap,
                                 ?string $encoding = null)[read_globals]: mixed;

/**
 * Detects character encoding in string str.
 *
 * @param string $str - The string being detected.
 * @param mixed $encoding_list - encoding_list is list of character encoding.
 *   Encoding order may be specified by array or comma separated list string. If
 *   encoding_list is omitted, detect_order is used.
 * @param mixed $strict - strict specifies whether to use the strict encoding
 *   detection or not. Default is FALSE.
 *
 * @return mixed - The detected character encoding or FALSE if the encoding
 *   cannot be detected from the given string.
 *
 */
<<__Native>>
function mb_detect_encoding(string $str,
                            mixed $encoding_list = null,
                            mixed $strict = null)[read_globals]: mixed;

/**
 * Sets the automatic character encoding detection order to encoding_list.
 *
 * @param mixed $encoding_list - encoding_list is an array or comma separated
 *   list of character encoding. ("auto" is expanded to "ASCII, JIS, UTF-8,
 *   EUC-JP, SJIS") If encoding_list is omitted, it returns the current
 *   character encoding detection order as array. This setting affects
 *   mb_detect_encoding() and mb_send_mail(). mbstring currently implements the
 *   following encoding detection filters. If there is an invalid byte sequence
 *   for the following encodings, encoding detection will fail. UTF-8, UTF-7,
 *   ASCII, EUC-JP,SJIS, eucJP-win, SJIS-win, JIS, ISO-2022-JP For ISO-8859-*,
 *   mbstring always detects as ISO-8859-*. For UTF-16, UTF-32, UCS2 and UCS4,
 *   encoding detection will fail always. Example #1 Useless detect order
 *   example
 *
 * @return mixed - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function mb_detect_order(mixed $encoding_list = null)[read_globals]: mixed;

/**
 * Encodes a given string str by the MIME header encoding scheme.
 *
 * @param string $str - The string being encoded.
 * @param string $charset - charset specifies the name of the character set in
 *   which str is represented in. The default value is determined by the current
 *   NLS setting (mbstring.language). mb_internal_encoding() should be set to
 *   same encoding.
 * @param string $transfer_encoding - transfer_encoding specifies the scheme
 *   of MIME encoding. It should be either "B" (Base64) or "Q"
 *   (Quoted-Printable). Falls back to "B" if not given.
 * @param string $linefeed
 * @param int $indent - Indentation of the first line (number of characters in
 *   the header before str).
 *
 * @return mixed - A converted version of the string represented in ASCII.
 *
 */
<<__Native>>
function mb_encode_mimeheader(string $str,
                              ?string $charset = null,
                              ?string $transfer_encoding = null,
                              string $linefeed = "\r\n",
                              int $indent = 0)[read_globals]: mixed;

/**
 * Converts specified character codes in string str from HTML numeric
 *   character reference to character code.
 *
 * @param string $str - The string being encoded.
 * @param mixed $convmap - convmap is array specifies code area to convert.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - The converted string.
 *
 */
<<__Native>>
function mb_encode_numericentity(string $str,
                                 mixed $convmap,
                                 ?string $encoding = null,
                                 bool $is_hex = false)[read_globals]: mixed;

/**
 * Get aliases of a known encoding type.
 *
 * @param string $encoding - The encoding type being checked, for aliases.
 *
 * @return mixed - Returns a numerically indexed array of encoding aliases on
 *   success, or FALSE on failure.
 *
 */
<<__Native>>
function mb_encoding_aliases(string $str)[]: mixed;

/**
 * A regular expression match for a multibyte string
 *
 * @param string $pattern - The regular expression pattern.
 * @param string $str - The string being evaluated.
 * @param string $option
 *
 * @return bool - Returns TRUE if string matches the regular expression
 *   pattern, FALSE if not.
 *
 */
<<__Native>>
function mb_ereg_match(string $pattern,
                       string $str,
                       ?string $option = null)[read_globals]: bool;

/**
 * @param mixed $pattern - The regular expression pattern. Multibyte
 *   characters may be used in pattern.
 * @param string $replacement - The replacement text.
 * @param string $str - The string being checked.
 * @param string $option - Matching condition can be set by option parameter.
 *   If i is specified for this parameter, the case will be ignored. If x is
 *   specified, white space will be ignored. If m is specified, match will be
 *   executed in multiline mode and line break will be included in '.'. If p is
 *   specified, match will be executed in POSIX mode, line break will be
 *   considered as normal character. If e is specified, replacement string will
 *   be evaluated as PHP expression.
 *
 * @return mixed - The resultant string on success, or FALSE on error.
 *
 */
<<__Native>>
function mb_ereg_replace(mixed $pattern,
                         string $replacement,
                         string $str,
                         ?string $option = null)[read_globals]: mixed;

/**
 * @return int - mb_ereg_search_getpos() returns the point to start regular
 *   expression match for mb_ereg_search(), mb_ereg_search_pos(),
 *   mb_ereg_search_regs(). The position is represented by bytes from the head
 *   of string.
 *
 */
<<__Native>>
function mb_ereg_search_getpos()[read_globals]: int;

/**
 * @return mixed - An array including the sub-string of matched part by last
 *   mb_ereg_search(), mb_ereg_search_pos(), mb_ereg_search_regs(). If there are
 *   some matches, the first element will have the matched sub-string, the
 *   second element will have the first part grouped with brackets, the third
 *   element will have the second part grouped with brackets, and so on. It
 *   returns FALSE on error;
 *
 */
<<__Native>>
function mb_ereg_search_getregs()[read_globals]: mixed;

/**
 * mb_ereg_search_init() sets string and pattern for a multibyte regular
 *   expression. These values are used for mb_ereg_search(),
 *   mb_ereg_search_pos(), and mb_ereg_search_regs().
 *
 * @param string $str - The search string.
 * @param string $pattern - The search pattern.
 * @param string $option - The search option.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function mb_ereg_search_init(string $str,
                             ?string $pattern = null,
                             ?string $option = null)[read_globals]: bool;

/**
 * Returns position and length of a matched part of the multibyte regular
 *   expression for a predefined multibyte string The string for match is
 *   specified by mb_ereg_search_init(). If it is not specified, the previous
 *   one will be used.
 *
 * @param string $pattern - The search pattern.
 * @param string $option - The search option.
 *
 * @return mixed - An array including the position of a matched part for a
 *   multibyte regular expression. The first element of the array will be the
 *   beginning of matched part, the second element will be length (bytes) of
 *   matched part. It returns FALSE on error.
 *
 */
<<__Native>>
function mb_ereg_search_pos(?string $pattern = null,
                            ?string $option = null)[read_globals]: mixed;

/**
 * Returns the matched part of a multibyte regular expression.
 *
 * @param string $pattern - The search pattern.
 * @param string $option - The search option.
 *
 * @return mixed - mb_ereg_search_regs() executes the multibyte regular
 *   expression match, and if there are some matched part, it returns an array
 *   including substring of matched part as first element, the first grouped
 *   part with brackets as second element, the second grouped part as third
 *   element, and so on. It returns FALSE on error.
 *
 */
<<__Native>>
function mb_ereg_search_regs(?string $pattern = null,
                             ?string $option = null)[read_globals]: mixed;

/**
 * @param int $position - The position to set.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function mb_ereg_search_setpos(int $position)[leak_safe]: bool;

/**
 * Performs a multibyte regular expression match for a predefined multibyte
 *   string.
 *
 * @param string $pattern - The search pattern.
 * @param string $option - The search option.
 *
 * @return mixed - mb_ereg_search() returns TRUE if the multibyte string
 *   matches with the regular expression, or FALSE otherwise. The string for
 *   matching is set by mb_ereg_search_init(). If pattern is not specified, the
 *   previous one is used.
 *
 */
<<__Native>>
function mb_ereg_search(?string $pattern = null, ?string $option = null)[read_globals]: mixed;

/**
 * @param mixed $pattern - The search pattern.
 * @param string $str - The search string.
 * @param mixed $regs - Contains a substring of the matched string.
 *
 * @return mixed - Executes the regular expression match with multibyte
 *   support, and returns 1 if matches are found. If the optional regs parameter
 *   was specified, the function returns the byte length of matched part, and
 *   the array regs will contain the substring of matched string. The function
 *   returns 1 if it matches with the empty string. If no matches are found or
 *   an error happens, FALSE will be returned.
 *
 */
<<__Native>>
function mb_ereg(mixed $pattern,
                 string $str,
                 <<__OutOnly>>
                 inout mixed $regs)[leak_safe]: mixed;

/**
 * @param mixed $pattern - The regular expression pattern. Multibyte
 *   characters may be used. The case will be ignored.
 * @param string $replacement - The replacement text.
 * @param string $str - The searched string.
 * @param string $option - option has the same meaning as in
 *   mb_ereg_replace().
 *
 * @return mixed - The resultant string or FALSE on error.
 *
 */
<<__Native>>
function mb_eregi_replace(mixed $pattern,
                          string $replacement,
                          string $str,
                          ?string $option = null)[leak_safe]: mixed;

/**
 * @param mixed $pattern - The regular expression pattern.
 * @param string $str - The string being searched.
 * @param mixed $regs - Contains a substring of the matched string.
 *
 * @return mixed - Executes the regular expression match with multibyte
 *   support, and returns 1 if matches are found. If the optional regs parameter
 *   was specified, the function returns the byte length of matched part, and
 *   the array regs will contain the substring of matched string. The function
 *   returns 1 if it matches with the empty string. If no matches are found or
 *   an error happens, FALSE will be returned.
 *
 */
<<__Native>>
function mb_eregi(mixed $pattern,
                  string $str,
                  <<__OutOnly>>
                  inout mixed $regs)[leak_safe]: mixed;

/**
 * @param string $type - If type isn't specified or is specified to "all", an
 *   array having the elements "internal_encoding", "http_output", "http_input",
 *   "func_overload", "mail_charset", "mail_header_encoding",
 *   "mail_body_encoding" will be returned. If type is specified as
 *   "http_output", "http_input", "internal_encoding", "func_overload", the
 *   specified setting parameter will be returned.
 *
 * @return mixed - An array of type information if type is not specified,
 *   otherwise a specific type.
 *
 */
<<__Native>>
function mb_get_info(?string $type = null)[read_globals]: mixed;

/**
 * @param string $type - Input string specifies the input type. "G" for GET,
 *   "P" for POST, "C" for COOKIE, "S" for string, "L" for list, and "I" for the
 *   whole list (will return array). If type is omitted, it returns the last
 *   input type processed.
 *
 * @return mixed - The character encoding name, as per the type. If
 *   mb_http_input() does not process specified HTTP input, it returns FALSE.
 *
 */
<<__Native>>
function mb_http_input(?string $type = null)[read_globals]: mixed;

/**
 * Set/Get the HTTP output character encoding. Output after this function is
 *   converted to encoding.
 *
 * @param string $encoding - If encoding is set, mb_http_output() sets the
 *   HTTP output character encoding to encoding. If encoding is omitted,
 *   mb_http_output() returns the current HTTP output character encoding.
 *
 * @return mixed - If encoding is omitted, mb_http_output() returns the
 *   current HTTP output character encoding. Otherwise, Returns TRUE on success
 *   or FALSE on failure.
 *
 */
<<__Native>>
function mb_http_output(?string $encoding = null)[leak_safe]: mixed;

/**
 * Set/Get the internal character encoding
 *
 * @param string $encoding - encoding is the character encoding name used for
 *   the HTTP input character encoding conversion, HTTP output character
 *   encoding conversion, and the default character encoding for string
 *   functions defined by the mbstring module.
 *
 * @return mixed - If encoding is set, then Returns TRUE on success or FALSE
 *   on failure. If encoding is omitted, then the current character encoding
 *   name is returned.
 *
 */
<<__Native>>
function mb_internal_encoding(?string $encoding = null)[leak_safe]: mixed;

/**
 * Set/Get the current language.
 *
 * @param string $language - Used for encoding e-mail messages. Valid
 *   languages are "Japanese", "ja","English","en" and "uni" (UTF-8).
 *   mb_send_mail() uses this setting to encode e-mail. Language and its setting
 *   is ISO-2022-JP/Base64 for Japanese, UTF-8/Base64 for uni, ISO-8859-1/quoted
 *   printable for English.
 *
 * @return mixed - If language is set and language is valid, it returns TRUE.
 *   Otherwise, it returns FALSE. When language is omitted, it returns the
 *   language name as a string. If no language is set previously, it then
 *   returns FALSE.
 *
 */
<<__Native>>
function mb_language(?string $language = null)[leak_safe]: mixed;

/**
 * mb_output_handler() is ob_start() callback function. mb_output_handler()
 *   converts characters in the output buffer from internal character encoding
 *   to HTTP output character encoding.
 *
 * @param string $contents - The contents of the output buffer.
 * @param int $status - The status of the output buffer.
 *
 * @return string - The converted string.
 *
 */
<<__Native>>
function mb_output_handler(string $contents, int $status)[leak_safe]: string;

/**
 * Parses GET/POST/COOKIE data and sets global variables. Since PHP does not
 *   provide raw POST/COOKIE data, it can only be used for GET data for now. It
 *   parses URL encoded data, detects encoding, converts coding to internal
 *   encoding and set values to the result array or global variables.
 *
 * @param string $encoded_string - The URL encoded data.
 * @param mixed $result - An array containing decoded and character encoded
 *   converted values.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function mb_parse_str(string $encoded_string,
                      <<__OutOnly("darray")>>
                      inout mixed $result)[leak_safe]: bool;

/**
 * Get a MIME charset string for a specific encoding.
 *
 * @param string $encoding - The encoding being checked.
 *
 * @return mixed - The MIME charset string for character encoding encoding.
 *
 */
<<__Native>>
function mb_preferred_mime_name(string $encoding)[]: mixed;

/**
 * Returns the current encoding for a multibyte regex as a string.
 *
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - Returns the character encoding used by multibyte regex
 *   functions.
 *
 */
<<__Native>>
function mb_regex_encoding(?string $encoding = null)[leak_safe]: mixed;

/**
 * @param string $options - The options to set. This is a string where each
 *   character is an option. To set a mode, the mode character must be the last
 *   one set, however there can only be set one mode but multiple options. Regex
 *   options Option Meaning i Ambiguity match on x Enables extended pattern form
 *   m '.' matches with newlines s '^' -> '\A', '$' -> '\Z' p Same as both the m
 *   and s options l Finds longest matches n Ignores empty matches e eval()
 *   resulting code Regex syntax modes Mode Meaning j Java (Sun java.util.regex)
 *   u GNU regex g grep c Emacs r Ruby z Perl b POSIX Basic regex d POSIX
 *   Extended regex
 *
 * @return string - The previous options. If options is omitted, it returns
 *   the string that describes the current options.
 *
 */
<<__Native>>
function mb_regex_set_options(?string $options = null)[leak_safe]: string;

/**
 * Sends email. Headers and messages are converted and encoded according to
 *   the mb_language() setting. It's a wrapper function for mail(), so see also
 *   mail() for details.
 *
 * @param string $to - The mail addresses being sent to. Multiple recipients
 *   may be specified by putting a comma between each address in to. This
 *   parameter is not automatically encoded.
 *
 * @param string $subject - The subject of the mail.
 *
 * @param string $message - The message of the mail.
 *
 * @param string $headers - additional_headers is inserted at the end of the
 *   header. This is typically used to add extra headers. Multiple extra headers
 *   are separated with a newline ("\n").
 *
 * @param string $extra_cmd - additional_parameter is a MTA command line
 *   parameter. It is useful when setting the correct Return-Path header when
 *   using sendmail.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function mb_send_mail(string $to,
                      string $subject,
                      string $message,
                      ?string $headers = null,
                      ?string $extra_cmd = null): bool;

/**
 * @param string $pattern - The regular expression pattern.
 *
 * @param string $str - The string being split.
 *
 * @param int $count - If optional parameter limit is specified, it will be
 *   split in limit elements as maximum.
 *
 * @return mixed - The result as an array.
 *
 */
<<__Native>>
function mb_split(string $pattern,
                  string $str,
                  int $count = -1)[leak_safe]: mixed;

/**
 * mb_strcut() performs equivalent operation as mb_substr() with different
 *   method. If start position is multi-byte character's second byte or larger,
 *   it starts from first byte of multi-byte character. It subtracts string from
 *   str that is shorter than length AND character that is not part of
 *   multi-byte string or not being middle of shift sequence.
 *
 * @param string $str - The string being cut.
 * @param int $start - The position that begins the cut.
 * @param mixed $length - The string being decoded.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - mb_strcut() returns the portion of str specified by the
 *   start and length parameters.
 *
 */
<<__Native>>
function mb_strcut(string $str,
                   int $start,
                   mixed $length = null,
                   ?string $encoding = null)[read_globals]: mixed;

/**
 * Truncates string str to specified width.
 *
 * @param string $str - The string being decoded.
 * @param int $start - The start position offset. Number of characters from
 *   the beginning of string. (First character is 0)
 * @param int $width - The width of the desired trim.
 * @param string $trimmarker - A string that is added to the end of string
 *   when string is truncated.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - The truncated string. If trimmarker is set, trimmarker is
 *   appended to the return value.
 *
 */
<<__Native>>
function mb_strimwidth(string $str,
                       int $start,
                       int $width,
                       ?string $trimmarker = null,
                       ?string $encoding = null)[read_globals]: mixed;

/**
 * mb_stripos() returns the numeric position of the first occurrence of needle
 *   in the haystack string. Unlike mb_strpos(), mb_stripos() is
 *   case-insensitive. If needle is not found, it returns FALSE.
 *
 * @param string $haystack - The string from which to get the position of the
 *   first occurrence of needle
 * @param string $needle - The string to find in haystack
 * @param int $offset - The position in haystack to start searching
 * @param string $encoding - Character encoding name to use. If it is omitted,
 *   internal character encoding is used.
 *
 * @return mixed - Return the numeric position of the first occurrence of
 *   needle in the haystack string, or FALSE if needle is not found.
 *
 */
<<__Native>>
function mb_stripos(string $haystack,
                    string $needle,
                    int $offset = 0,
                    ?string $encoding = null)[read_globals]: mixed;

/**
 * mb_stristr() finds the first occurrence of needle in haystack and returns
 *   the portion of haystack. Unlike mb_strstr(), mb_stristr() is
 *   case-insensitive. If needle is not found, it returns FALSE.
 *
 * @param string $haystack - The string from which to get the first occurrence
 *   of needle
 * @param string $needle - The string to find in haystack
 * @param bool $part - Determines which portion of haystack this function
 *   returns. If set to TRUE, it returns all of haystack from the beginning to
 *   the first occurrence of needle. If set to FALSE, it returns all of haystack
 *   from the first occurrence of needle to the end,
 * @param string $encoding - Character encoding name to use. If it is omitted,
 *   internal character encoding is used.
 *
 * @return mixed - Returns the portion of haystack, or FALSE if needle is not
 *   found.
 *
 */
<<__Native>>
function mb_stristr(string $haystack,
                    string $needle,
                    bool $part = false,
                    ?string $encoding = null)[read_globals]: mixed;

/**
 * Gets the length of a string.
 *
 * @param string $str - The string being checked for length.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - Returns the number of characters in string str having
 *   character encoding encoding. A multi-byte character is counted as 1.
 *
 */
<<__Native>>
function mb_strlen(string $str, ?string $encoding = null)[read_globals]: mixed;

/**
 * Finds position of the first occurrence of a string in a string. Performs a
 *   multi-byte safe strpos() operation based on number of characters. The first
 *   character's position is 0, the second character position is 1, and so on.
 *
 * @param string $haystack - The string being checked.
 * @param string $needle - The position counted from the beginning of
 *   haystack.
 * @param int $offset - The search offset. If it is not specified, 0 is used.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - Returns the numeric position of the first occurrence of
 *   needle in the haystack string. If needle is not found, it returns FALSE.
 *
 */
<<__Native>>
function mb_strpos(string $haystack,
                   string $needle,
                   int $offset = 0,
                   ?string $encoding = null)[read_globals]: mixed;

/**
 * mb_strrchr() finds the last occurrence of needle in haystack and returns
 *   the portion of haystack. If needle is not found, it returns FALSE.
 *
 * @param string $haystack - The string from which to get the last occurrence
 *   of needle
 * @param string $needle - The string to find in haystack
 * @param bool $part - Determines which portion of haystack this function
 *   returns. If set to TRUE, it returns all of haystack from the beginning to
 *   the last occurrence of needle. If set to FALSE, it returns all of haystack
 *   from the last occurrence of needle to the end,
 * @param string $encoding - Character encoding name to use. If it is omitted,
 *   internal character encoding is used.
 *
 * @return mixed - Returns the portion of haystack. or FALSE if needle is not
 *   found.
 *
 */
<<__Native>>
function mb_strrchr(string $haystack,
                    string $needle,
                    bool $part = false,
                    ?string $encoding = null)[read_globals]: mixed;

/**
 * mb_strrichr() finds the last occurrence of needle in haystack and returns
 *   the portion of haystack. Unlike mb_strrchr(), mb_strrichr() is
 *   case-insensitive. If needle is not found, it returns FALSE.
 *
 * @param string $haystack - The string from which to get the last occurrence
 *   of needle
 * @param string $needle - The string to find in haystack
 * @param bool $part - Determines which portion of haystack this function
 *   returns. If set to TRUE, it returns all of haystack from the beginning to
 *   the last occurrence of needle. If set to FALSE, it returns all of haystack
 *   from the last occurrence of needle to the end,
 * @param string $encoding - Character encoding name to use. If it is omitted,
 *   internal character encoding is used.
 *
 * @return mixed - Returns the portion of haystack. or FALSE if needle is not
 *   found.
 *
 */
<<__Native>>
function mb_strrichr(string $haystack,
                     string $needle,
                     bool $part = false,
                     ?string $encoding = null)[read_globals]: mixed;

/**
 * mb_strripos() performs multi-byte safe strripos() operation based on number
 *   of characters. needle position is counted from the beginning of haystack.
 *   First character's position is 0. Second character position is 1. Unlike
 *   mb_strrpos(), mb_strripos() is case-insensitive.
 *
 * @param string $haystack - The string from which to get the position of the
 *   last occurrence of needle
 * @param string $needle - The string to find in haystack
 * @param int $offset - The position in haystack to start searching
 * @param string $encoding - Character encoding name to use. If it is omitted,
 *   internal character encoding is used.
 *
 * @return mixed - Return the numeric position of the last occurrence of
 *   needle in the haystack string, or FALSE if needle is not found.
 *
 */
<<__Native>>
function mb_strripos(string $haystack,
                     string $needle,
                     int $offset = 0,
                     ?string $encoding = null)[read_globals]: mixed;

/**
 * Performs a multibyte safe strrpos() operation based on the number of
 *   characters. needle position is counted from the beginning of haystack.
 *   First character's position is 0. Second character position is 1.
 *
 * @param string $haystack - The string being checked, for the last occurrence
 *   of needle
 * @param string $needle - The string to find in haystack.
 * @param mixed $offset - May be specified to begin searching an arbitrary
 *   number of characters into the string. Negative values will stop searching
 *   at an arbitrary point prior to the end of the string.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - Returns the numeric position of the last occurrence of
 *   needle in the haystack string. If needle is not found, it returns FALSE.
 *
 */
<<__Native>>
function mb_strrpos(string $haystack,
                    string $needle,
                    mixed $offset = 0,
                    ?string $encoding = null)[read_globals]: mixed;

/**
 * mb_strstr() finds the first occurrence of needle in haystack and returns
 *   the portion of haystack. If needle is not found, it returns FALSE.
 *
 * @param string $haystack - The string from which to get the first occurrence
 *   of needle
 * @param string $needle - The string to find in haystack
 * @param bool $part - Determines which portion of haystack this function
 *   returns. If set to TRUE, it returns all of haystack from the beginning to
 *   the first occurrence of needle. If set to FALSE, it returns all of haystack
 *   from the first occurrence of needle to the end,
 * @param string $encoding - Character encoding name to use. If it is omitted,
 *   internal character encoding is used.
 *
 * @return mixed - Returns the portion of haystack, or FALSE if needle is not
 *   found.
 *
 */
<<__Native>>
function mb_strstr(string $haystack,
                   string $needle,
                   bool $part = false,
                   ?string $encoding = null)[read_globals]: mixed;

/**
 * Returns str with all alphabetic characters converted to lowercase.
 *
 * @param string $str - The string being lowercased.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - str with all alphabetic characters converted to lowercase.
 *
 */
<<__Native>>
function mb_strtolower(string $str, ?string $encoding = null)[read_globals]: mixed;

/**
 * Returns str with all alphabetic characters converted to uppercase.
 *
 * @param string $str - The string being uppercased.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - str with all alphabetic characters converted to uppercase.
 *
 */
<<__Native>>
function mb_strtoupper(string $str, ?string $encoding = null)[read_globals]: mixed;

/**
 * Returns the width of string str. Multi-byte characters are usually twice
 *   the width of single byte characters. Characters width Chars Width U+0000 -
 *   U+0019 0 U+0020 - U+1FFF 1 U+2000 - U+FF60 2 U+FF61 - U+FF9F 1 U+FFA0 - 2
 *
 * @param string $str - The string being decoded.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - The width of string str.
 *
 */
<<__Native>>
function mb_strwidth(string $str, ?string $encoding = null)[read_globals]: mixed;

/**
 * Specifies a substitution character when input character encoding is invalid
 *   or character code does not exist in output character encoding. Invalid
 *   characters may be substituted NULL (no output), string or integer value
 *   (Unicode character code value). This setting affects mb_convert_encoding(),
 *   mb_convert_variables(), mb_output_handler(), and mb_send_mail().
 *
 * @param mixed $substrchar - Specify the Unicode value as an integer, or as
 *   one of the following strings: "none" : no output "long" : Output character
 *   code value (Example: U+3000, JIS+7E7E) "entity" : Output character entity
 *
 * @return mixed - If substchar is set, it returns TRUE for success, otherwise
 *   returns FALSE. If substchar is not set, it returns the Unicode value, or
 *   "none" or "long".
 *
 */
<<__Native>>
function mb_substitute_character(mixed $substrchar = null)[leak_safe]: mixed;

/**
 * Counts the number of times the needle substring occurs in the haystack
 *   string.
 *
 * @param string $haystack - The string being checked.
 * @param string $needle - The string being found.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - The number of times the needle substring occurs in the
 *   haystack string.
 *
 */
<<__Native>>
function mb_substr_count(string $haystack,
                         string $needle,
                         ?string $encoding = null)[read_globals]: mixed;

/**
 * Performs a multi-byte safe substr() operation based on number of
 *   characters. Position is counted from the beginning of str. First
 *   character's position is 0. Second character position is 1, and so on.
 *
 * @param string $str - The string being checked.
 * @param int $start - The first position used in str.
 * @param mixed $length - The maximum length of the returned string.
 * @param string $encoding - encoding parameter is the character encoding. If
 *   it is omitted, the internal character encoding value will be used.
 *
 * @return mixed - mb_substr() returns the portion of str specified by the
 *   start and length parameters.
 *
 */
<<__Native>>
function mb_substr(string $str,
                   int $start,
                   mixed $length = null,
                   ?string $encoding = null)[read_globals]: mixed;
