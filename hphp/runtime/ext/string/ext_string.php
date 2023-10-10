<?hh

namespace HH {
newtype FormatString<T> = string;
}

namespace {

// These are ostensibly bools,
// but for historical reasons are expressed as ints
const int CRYPT_BLOWFISH = 1;
const int CRYPT_EXT_DES = 0;
const int CRYPT_MD5 = 1;
const int CRYPT_STD_DES = 1;

const int CRYPT_SALT_LENGTH = 12;

/**
 * Returns a string with backslashes before characters that are listed in
 *   charlist parameter.
 *
 * @param string $str - The string to be escaped.
 * @param string $charlist - A list of characters to be escaped. If charlist
 *   contains characters \n, \r etc., they are converted in C-like style, while
 *   other non-alphanumeric characters with ASCII codes lower than 32 and higher
 *   than 126 converted to octal representation.  When you define a sequence of
 *   characters in the charlist argument make sure that you know what characters
 *   come between the characters that you set as the start and end of the range.
 *    Also, if the first character in a range has a higher ASCII value than the
 *   second character in the range, no range will be constructed. Only the
 *   start, end and period characters will be escaped. Use the ord() function to
 *   find the ASCII value for a character.    Be careful if you choose to escape
 *   characters 0, a, b, f, n, r, t and v. They will be converted to \0, \a, \b,
 *   \f, \n, \r, \t and \v. In PHP \0 (NULL), \r (carriage return), \n
 *   (newline), \f (form feed), \v (vertical tab) and \t (tab) are predefined
 *   escape sequences, while in C all of these are predefined escape sequences.
 *
 * @return string - Returns the escaped string.
 *
 */
<<__IsFoldable, __Native>>
function addcslashes(string $str, string $charlist)[]: string;

/**
 * Returns a string with backslashes stripped off. Recognizes C-like \n, \r
 *   ..., octal and hexadecimal representation.
 *
 * @param string $str - The string to be unescaped.
 *
 * @return string - Returns the unescaped string.
 *
 */
<<__IsFoldable, __Native>>
function stripcslashes(string $str)[]: string;

/**
 * Returns a string with backslashes before characters that need to be quoted
 *   in database queries etc. These characters are single quote ('), double
 *   quote ("), backslash (\) and NUL (the NULL byte).  An example use of
 *   addslashes() is when you're entering data into a database. For example, to
 *   insert the name O'reilly into a database, you will need to escape it. It's
 *   highly recommended to use DBMS specific escape function (e.g.
 *   mysqli_real_escape_string() for MySQL or pg_escape_string() for
 *   PostgreSQL), but if the DBMS you're using doesn't have an escape function
 *   and the DBMS uses \ to escape special chars, you can use this function.
 *   This would only be to get the data into the database, the extra \ will not
 *   be inserted. Having the PHP directive magic_quotes_sybase set to on will
 *   mean ' is instead escaped with another '.  The PHP directive
 *   magic_quotes_gpc is on by default, and it essentially runs addslashes() on
 *   all GET, POST, and COOKIE data. Do not use addslashes() on strings that
 *   have already been escaped with magic_quotes_gpc as you'll then do double
 *   escaping. The function get_magic_quotes_gpc() may come in handy for
 *   checking this.
 *
 * @param string $str - The string to be escaped.
 *
 * @return string - Returns the escaped string.
 *
 */
<<__IsFoldable, __Native>>
function addslashes(string $str)[]: string;

/**
 * Un-quotes a quoted string.  If magic_quotes_sybase is on, no backslashes
 *   are stripped off but two apostrophes are replaced by one instead.  An
 *   example use of stripslashes() is when the PHP directive magic_quotes_gpc is
 *   on (it's on by default), and you aren't inserting this data into a place
 *   (such as a database) that requires escaping. For example, if you're simply
 *   outputting data straight from an HTML form.
 *
 * @param string $str - The input string.
 *
 * @return string - Returns a string with backslashes stripped off. (\'
 *   becomes ' and so on.) Double backslashes (\\) are made into a single
 *   backslash (\).
 *
 */
<<__IsFoldable, __Native>>
function stripslashes(string $str)[]: string;

/**
 * Returns an ASCII string containing the hexadecimal representation of str.
 *   The conversion is done byte-wise with the high-nibble first.
 *
 * @param string $str - A character.
 *
 * @return string - Returns the hexadecimal representation of the given
 *   string.
 *
 */
<<__IsFoldable, __Native>>
function bin2hex(string $str)[]: string;

/**
 * Returns an ASCII string containing the binary representation of hexidecimal
 *   str.
 *
 * @param string $str - A character.
 *
 * @return mixed - Returns the binary representation of the given hexidecimal
 *   string or FALSE on failure.
 *
 */
<<__IsFoldable, __Native>>
function hex2bin(string $str)[]: mixed;

/**
 * Returns string with '<br />' or '<br>' inserted before all newlines.
 *
 * @param string $str - The input string.
 * @param bool $is_xhtml - Whether to use XHTML compatible line breaks or not.
 *
 * @return string - Returns the altered string.
 *
 */
<<__IsFoldable, __Native>>
function nl2br(string $str, bool $is_xhtml = true)[]: string;

/**
 * Returns a version of str with a backslash character (\) before every
 *   character that is among these: . \ + * ? [ ^ ] ( $ )
 *
 * @param string $str - The input string.
 *
 * @return string - Returns the string with meta characters quoted.
 *
 */
<<__IsFoldable, __Native>>
function quotemeta(string $str)[]: string;

/**
 * @param string $str - The input string.
 *
 * @return string - Returns the shuffled string.
 *
 */
<<__Native>>
function str_shuffle(string $str): string;

/**
 * Returns string, reversed.
 *
 * @param string $str - The string to be reversed.
 *
 * @return string - Returns the reversed string.
 *
 */
<<__IsFoldable, __Native>>
function strrev(string $str)[]: string;

/**
 * Returns string with all alphabetic characters converted to lowercase.  Note
 *   that 'alphabetic' is determined by the current locale. This means that in
 *   e.g. the default "C" locale, characters such as umlaut-A will not be
 *   converted.
 *
 * @param string $str - The input string.
 *
 * @return string - Returns the lowercased string.
 *
 */
<<__Native>>
function strtolower(string $str)[]: string;

/**
 * Returns string with all alphabetic characters converted to uppercase.  Note
 *   that 'alphabetic' is determined by the current locale. For instance, in the
 *   default "C" locale characters such as umlaut-a will not be converted.
 *
 * @param string $str - The input string.
 *
 * @return string - Returns the uppercased string.
 *
 */
<<__Native>>
function strtoupper(string $str)[]: string;

/**
 * Returns a string with the first character of str capitalized, if that
 *   character is alphabetic.  Note that 'alphabetic' is determined by the
 *   current locale. For instance, in the default "C" locale characters such as
 *   umlaut-a will not be converted.
 *
 * @param string $str - The input string.
 *
 * @return string - Returns the resulting string.
 *
 */
<<__Native>>
function ucfirst(string $str): string;

/**
 * Returns a string with the first character of str , lowercased if that
 *   character is alphabetic.  Note that 'alphabetic' is determined by the
 *   current locale. For instance, in the default "C" locale characters such as
 *   umlaut-a will not be converted.
 *
 * @param string $str - The input string.
 *
 * @return string - Returns the resulting string.
 *
 */
<<__Native>>
function lcfirst(string $str): string;

/**
 * Returns a string with the first character of each word in str capitalized,
 *   if that character is alphabetic.  The definition of a word is any string of
 *   characters that is immediately after a whitespace (These are: space,
 *   form-feed, newline, carriage return, horizontal tab, and vertical tab).
 *
 * @param string $str - The input string.
 *
 * @return string - Returns the modified string.
 *
 */
<<__Native>>
function ucwords(string $str, string $delimiters = " \t\r\n\f\v"): string;

/**
 * This function tries to return a string with all NUL bytes, HTML and PHP
 *   tags stripped from a given str. It uses the same tag stripping state
 *   machine as the fgetss() function.
 *
 * @param string $str - The input string.
 * @param string $allowable_tags - You can use the optional second parameter
 *   to specify tags which should not be stripped.  HTML comments and PHP tags
 *   are also stripped. This is hardcoded and can not be changed with
 *   allowable_tags.
 *
 * @return string - Returns the stripped string.
 *
 */
<<__Native>>
function strip_tags(string $str, mixed $allowable_tags = ""): string;

/**
 * This function returns a string with whitespace stripped from the beginning
 *   and end of str. Without the second parameter, trim() will strip these
 *   characters: " " (ASCII 32 (0x20)), an ordinary space. "\t" (ASCII 9
 *   (0x09)), a tab. "\n" (ASCII 10 (0x0A)), a new line (line feed). "\r" (ASCII
 *   13 (0x0D)), a carriage return. "\0" (ASCII 0 (0x00)), the NUL-byte. "\x0B"
 *   (ASCII 11 (0x0B)), a vertical tab.
 *
 * @param string $str - The string that will be trimmed.
 * @param string $charlist - Optionally, the stripped characters can also be
 *   specified using the charlist parameter. Simply list all characters that you
 *   want to be stripped. With .. you can specify a range of characters.
 *
 * @return string - The trimmed string.
 *
 */
<<__IsFoldable, __Native>>
function trim(string $str, string $charlist = HPHP_TRIM_CHARLIST)[]: string;

/**
 * Strip whitespace (or other characters) from the beginning of a string.
 *
 * @param string $str - The input string.
 * @param string $charlist - You can also specify the characters you want to
 *   strip, by means of the charlist parameter. Simply list all characters that
 *   you want to be stripped. With .. you can specify a range of characters.
 *
 * @return string - This function returns a string with whitespace stripped
 *   from the beginning of str. Without the second parameter, ltrim() will strip
 *   these characters: " " (ASCII 32 (0x20)), an ordinary space. "\t" (ASCII 9
 *   (0x09)), a tab. "\n" (ASCII 10 (0x0A)), a new line (line feed). "\r" (ASCII
 *   13 (0x0D)), a carriage return. "\0" (ASCII 0 (0x00)), the NUL-byte. "\x0B"
 *   (ASCII 11 (0x0B)), a vertical tab.
 *
 */
<<__IsFoldable, __Native>>
function ltrim(string $str, string $charlist = HPHP_TRIM_CHARLIST)[]: string;

/**
 * This function returns a string with whitespace stripped from the end of
 *   str.  Without the second parameter, rtrim() will strip these characters: "
 *   " (ASCII 32 (0x20)), an ordinary space. "\t" (ASCII 9 (0x09)), a tab. "\n"
 *   (ASCII 10 (0x0A)), a new line (line feed). "\r" (ASCII 13 (0x0D)), a
 *   carriage return. "\0" (ASCII 0 (0x00)), the NUL-byte. "\x0B" (ASCII 11
 *   (0x0B)), a vertical tab.
 *
 * @param string $str - The input string.
 * @param string $charlist - You can also specify the characters you want to
 *   strip, by means of the charlist parameter. Simply list all characters that
 *   you want to be stripped. With .. you can specify a range of characters.
 *
 * @return string - Returns the modified string.
 *
 */
<<__IsFoldable, __Native>>
function rtrim(string $str, string $charlist = HPHP_TRIM_CHARLIST)[]: string;

<<__IsFoldable, __Native>>
function chop(string $str, string $charlist = HPHP_TRIM_CHARLIST)[]: string;

/**
 * Returns an array of strings, each of which is a substring of string formed
 *   by splitting it on boundaries formed by the string delimiter. Although
 *   implode() can, for historical reasons, accept its parameters in either
 *   order, explode() cannot. You must ensure that the delimiter argument comes
 *   before the string argument.
 *
 * @param string $delimiter - The boundary string.
 * @param string $str - The input string.
 * @param int $limit - If limit is set and positive, the returned array will
 *   contain a maximum of limit elements with the last element containing the
 *   rest of string.  If the limit parameter is negative, all components except
 *   the last -limit are returned.  If the limit parameter is zero, then this is
 *   treated as 1.
 *
 * @return mixed - Returns an array of strings created by splitting the string
 *   parameter on boundaries formed by the delimiter.  If delimiter is an empty
 *   string (""), explode() will return FALSE. If delimiter contains a value
 *   that is not contained in string and a negative limit is used, then an empty
 *   arraywill be returned, otherwise an array containing string will be
 *   returned.
 *
 */
<<__IsFoldable, __Native>>
function explode(string $delimiter,
                 string $str,
                 int $limit = 0x7FFFFFFF)[]: mixed;

/**
 * Join container elements with a glue string.  implode() can, for historical
 *   reasons, accept its parameters in either order. For consistency with
 *   explode(), however, it may be less confusing to use the documented order of
 *   arguments.
 *
 * @param mixed $arg1 - Defaults to an empty string. This is not the preferred
 *   usage of implode() as glue would be the second parameter and thus, the bad
 *   prototype would be used.
 * @param mixed $arg2 - The array of strings to implode.
 *
 * @return string - Returns a string containing a string representation of all
 *   the array elements in the same order, with the glue string between each
 *   element.
 *
 */
<<__IsFoldable, __Native>>
function implode(readonly mixed $arg1, readonly mixed $arg2 = null)[]: string;

/**
 * An alias for implode().
 *
 */
<<__IsFoldable, __Native>>
function join(mixed $arg1, mixed $arg2 = null)[]: string;

/**
 * Converts a string to an array.
 *
 * @param string $str - The input string.
 * @param int $split_length - Maximum length of the chunk.
 *
 * @return mixed - If the optional split_length parameter is specified, the
 *   returned array will be broken down into chunks with each being split_length
 *   in length, otherwise each chunk will be one character in length.  FALSE is
 *   returned if split_length is less than 1. If the split_length length exceeds
 *   the length of string, the entire string is returned as the first (and only)
 *   array element.
 *
 */
<<__IsFoldable, __Native>>
function str_split(string $str, int $split_length = 1)[]: mixed;

/**
 * Can be used to split a string into smaller chunks which is useful for e.g.
 *   converting base64_encode() output to match RFC 2045 semantics. It inserts
 *   end every chunklen characters.
 *
 * @param string $body - The string to be chunked.
 * @param int $chunklen - The chunk length.
 * @param string $end - The line ending sequence.
 *
 * @return mixed - Returns the chunked string.
 *
 */
<<__Native, __IsFoldable>>
function chunk_split(string $body,
                     int $chunklen = 76,
                     string $end = "\r\n")[]: mixed;

/**
 * strtok() splits a string (str) into smaller strings (tokens), with each
 *   token being delimited by any character from token. That is, if you have a
 *   string like "This is an example string" you could tokenize this string into
 *   its individual words by using the space character as the token.  Note that
 *   only the first call to strtok uses the string argument. Every subsequent
 *   call to strtok only needs the token to use, as it keeps track of where it
 *   is in the current string. To start over, or to tokenize a new string you
 *   simply call strtok with the string argument again to initialize it. Note
 *   that you may put multiple tokens in the token parameter. The string will be
 *   tokenized when any one of the characters in the argument are found.
 *
 * @param string $str - The string being split up into smaller strings
 *   (tokens).
 * @param mixed $token - The delimiter used when splitting up str.
 *
 * @return mixed - A string token.
 *
 */
<<__Native>>
function strtok(string $str, mixed $token = null): mixed;

/**
 * This function returns a string or an array with all occurrences of search
 *   in subject replaced with the given replace value.  If you don't need fancy
 *   replacing rules (like regular expressions), you should always use this
 *   function instead of ereg_replace() or preg_replace(). If search and replace
 *   are arrays, then str_replace() takes a value from each array and uses them
 *   to do search and replace on subject. If replace has fewer values than
 *   search, then an empty string is used for the rest of replacement values. If
 *   search is an array and replace is a string, then this replacement string is
 *   used for every value of search. The converse would not make sense, though.
 *   If search or replace are arrays, their elements are processed first to
 *   last.
 *
 * @param mixed $search - The value being searched for, otherwise known as the
 *   needle. An array may be used to designate multiple needles.
 * @param mixed $replace - The replacement value that replaces found search
 *   values. An array may be used to designate multiple replacements.
 * @param mixed $subject - The string or array being searched and replaced on,
 *   otherwise known as the haystack.  If subject is an array, then the search
 *   and replace is performed with every entry of subject, and the return value
 *   is an array as well.
 * @param mixed $count - If passed, this will hold the number of matched and
 *   replaced needles.
 *
 * @return mixed - This function returns a string or an array with the
 *   replaced values.
 *
 */
<<__IsFoldable, __Native>>
function str_replace(mixed $search,
                     mixed $replace,
                     mixed $subject)[]: mixed;

<<__IsFoldable, __Native>>
function str_replace_with_count(mixed $search,
                                mixed $replace,
                                mixed $subject,
                                <<__OutOnly("KindOfInt64")>>
                                inout mixed $count): mixed;

/**
 * This function returns a string or an array with all occurrences of search
 *   in subject (ignoring case) replaced with the given replace value. If you
 *   don't need fancy replacing rules, you should generally use this function
 *   instead of preg_replace() with the i modifier. If search and replace are
 *   arrays, then str_ireplace() takes a value from each array and uses them to
 *   do search and replace on subject. If replace has fewer values than search,
 *   then an empty string is used for the rest of replacement values. If search
 *   is an array and replace is a string, then this replacement string is used
 *   for every value of search.
 *
 * @param mixed $search - Every replacement with search array is performed on
 *   the result of previous replacement.
 * @param mixed $replace
 * @param mixed $subject - If subject is an array, then the search and replace
 *   is performed with every entry of subject, and the return value is an array
 *   as well.
 * @param mixed $count - The number of matched and replaced needles will be
 *   returned in count which is passed by reference.
 *
 * @return mixed - Returns a string or an array of replacements.
 *
 */
<<__IsFoldable, __Native>>
function str_ireplace(mixed $search,
                      mixed $replace,
                      mixed $subject): mixed;

<<__IsFoldable, __Native>>
function str_ireplace_with_count(mixed $search,
                                 mixed $replace,
                                 mixed $subject,
                                 <<__OutOnly("KindOfInt64")>>
                                 inout mixed $count): mixed;

/**
 * substr_replace() replaces a copy of string delimited by the start and
 *   (optionally) length parameters with the string given in replacement.
 *
 * @param mixed $str - The input string.
 * @param mixed $replacement - The replacement string.
 * @param mixed $start - If start is positive, the replacing will begin at the
 *   start'th offset into string.  If start is negative, the replacing will
 *   begin at the start'th character from the end of string.
 * @param mixed $length - If given and is positive, it represents the length
 *   of the portion of string which is to be replaced. If it is negative, it
 *   represents the number of characters from the end of string at which to stop
 *   replacing. If it is not given, then it will default to strlen( string );
 *   i.e. end the replacing at the end of string. Of course, if length is zero
 *   then this function will have the effect of inserting replacement into
 *   string at the given start offset.
 *
 * @return mixed - The result string is returned. If string is an array then
 *   array is returned.
 *
 */
<<__IsFoldable, __Native>>
function substr_replace(mixed $str,
                        mixed $replacement,
                        mixed $start,
                        mixed $length = 0x7FFFFFFF)[]: mixed;

/**
 * Returns the portion of string specified by the start and length parameters.
 *
 * @param string $str - The input string.
 * @param int $start - If start is non-negative, the returned string will
 *   start at the start'th position in string, counting from zero. For instance,
 *   in the string 'abcdef', the character at position 0 is 'a', the character
 *   at position 2 is 'c', and so forth.  If start is negative, the returned
 *   string will start at the start'th character from the end of string.  If
 *   string is less than or equal to start characters long, FALSE will be
 *   returned.  Example #1 Using a negative start
 * @param int $length - If length is given and is positive, the string
 *   returned will contain at most length characters beginning from start
 *   (depending on the length of string).  If length is given and is negative,
 *   then that many characters will be omitted from the end of string (after the
 *   start position has been calculated when a start is negative). If start
 *   denotes a position beyond this truncation, an empty string will be
 *   returned.  If length is given and is 0, FALSE or NULL an empty string will
 *   be returned.  If length is omitted, the substring starting from start until
 *   the end of the string will be returned. Example #2 Using a negative length
 *
 * @return mixed - Returns the extracted part of string or FALSE on failure.
 *
 */
<<__IsFoldable, __Native>>
function substr(string $str, int $start, int $length = 0x7FFFFFFF)[]: mixed;

/**
 * This functions returns the input string padded on the left, the right, or
 *   both sides to the specified padding length. If the optional argument
 *   pad_string is not supplied, the input is padded with spaces, otherwise it
 *   is padded with characters from pad_string up to the limit.
 *
 * @param string $input - The input string.
 * @param int $pad_length - If the value of pad_length is negative, less than,
 *   or equal to the length of the input string, no padding takes place.
 * @param string $pad_string - The pad_string may be truncated if the required
 *   number of padding characters can't be evenly divided by the pad_string's
 *   length.
 * @param int $pad_type - Optional argument pad_type can be STR_PAD_RIGHT,
 *   STR_PAD_LEFT, or STR_PAD_BOTH. If pad_type is not specified it is assumed
 *   to be STR_PAD_RIGHT.
 *
 * @return string - Returns the padded string.
 *
 */
<<__IsFoldable, __Native>>
function str_pad(string $input,
                 int $pad_length,
                 string $pad_string = " ",
                 int $pad_type = STR_PAD_RIGHT)[]: string;

/**
 * Returns input repeated multiplier times.
 *
 * @param string $input - The string to be repeated.
 *
 * @param int $multiplier - Number of time the input string should be
 *   repeated.  multiplier has to be greater than or equal to 0. If the
 *   multiplier is set to 0, the function will return an empty string.
 *
 * @return string - Returns the repeated string.
 *
 */
<<__IsFoldable, __Native>>
function str_repeat(string $input, int $multiplier)[]: string;

/**
 * html_entity_decode() is the opposite of htmlentities() in that it converts
 *   all HTML entities to their applicable characters from string.
 *
 * @param string $str - The input string.
 * @param int $quote_style - The optional second quote_style parameter lets
 *   you define what will be done with 'single' and "double" quotes. It takes on
 *   one of three constants with the default being ENT_COMPAT: Available
 *   quote_style constants Constant Name Description ENT_COMPAT Will convert
 *   double-quotes and leave single-quotes alone. ENT_QUOTES Will convert both
 *   double and single quotes. ENT_NOQUOTES Will leave both double and single
 *   quotes unconverted.
 * @param string $charset - The ISO-8859-1 character set is used as default
 *   for the optional third charset. This defines the character set used in
 *   conversion.  Following character sets are supported in PHP 4.3.0 and later.
 *   Supported charsets Charset Aliases Description ISO-8859-1 ISO8859-1 Western
 *   European, Latin-1 ISO-8859-15 ISO8859-15 Western European, Latin-9. Adds
 *   the Euro sign, French and Finnish letters missing in Latin-1(ISO-8859-1).
 *   UTF-8  ASCII compatible multi-byte 8-bit Unicode. cp866 ibm866, 866
 *   DOS-specific Cyrillic charset. This charset is supported in 4.3.2. cp1251
 *   Windows-1251, win-1251, 1251 Windows-specific Cyrillic charset. This
 *   charset is supported in 4.3.2. cp1252 Windows-1252, 1252 Windows specific
 *   charset for Western European. KOI8-R koi8-ru, koi8r Russian. This charset
 *   is supported in 4.3.2. BIG5 950 Traditional Chinese, mainly used in Taiwan.
 *   GB2312 936 Simplified Chinese, national standard character set. BIG5-HKSCS
 *   Big5 with Hong Kong extensions, Traditional Chinese. Shift_JIS SJIS, 932
 *   Japanese EUC-JP EUCJP Japanese Any other character sets are not recognized
 *   and ISO-8859-1 will be used instead.
 *
 * @return string - Returns the decoded string.
 *
 */
<<__Native>>
function html_entity_decode(string $str,
                            int $quote_style = ENT_COMPAT,
                            string $charset = "UTF-8")[]: string;

/**
 * This function is identical to htmlspecialchars() in all ways, except with
 *   htmlentities(), all characters which have HTML character entity equivalents
 *   are translated into these entities.  If you're wanting to decode instead
 *   (the reverse) you can use html_entity_decode().
 *
 * @param string $str - The input string.
 * @param int $quote_style - Like htmlspecialchars(), the optional second
 *   quote_style parameter lets you define what will be done with 'single' and
 *   "double" quotes. It takes on one of three constants with the default being
 *   ENT_COMPAT: Available quote_style constants Constant Name Description
 *   ENT_COMPAT Will convert double-quotes and leave single-quotes alone.
 *   ENT_QUOTES Will convert both double and single quotes. ENT_NOQUOTES Will
 *   leave both double and single quotes unconverted.
 * @param string $charset - Like htmlspecialchars(), it takes an optional
 *   third argument charset which defines character set used in conversion.
 *   Presently, the ISO-8859-1 character set is used as the default.  Following
 *   character sets are supported in PHP 4.3.0 and later. Supported charsets
 *   Charset Aliases Description ISO-8859-1 ISO8859-1 Western European, Latin-1
 *   ISO-8859-15 ISO8859-15 Western European, Latin-9. Adds the Euro sign,
 *   French and Finnish letters missing in Latin-1(ISO-8859-1). UTF-8  ASCII
 *   compatible multi-byte 8-bit Unicode. cp866 ibm866, 866 DOS-specific
 *   Cyrillic charset. This charset is supported in 4.3.2. cp1251 Windows-1251,
 *   win-1251, 1251 Windows-specific Cyrillic charset. This charset is supported
 *   in 4.3.2. cp1252 Windows-1252, 1252 Windows specific charset for Western
 *   European. KOI8-R koi8-ru, koi8r Russian. This charset is supported in
 *   4.3.2. BIG5 950 Traditional Chinese, mainly used in Taiwan. GB2312 936
 *   Simplified Chinese, national standard character set. BIG5-HKSCS  Big5 with
 *   Hong Kong extensions, Traditional Chinese. Shift_JIS SJIS, 932 Japanese
 *   EUC-JP EUCJP Japanese Any other character sets are not recognized and
 *   ISO-8859-1 will be used instead.
 * @param bool $double_encode - When double_encode is turned off PHP will not
 *   encode existing html entities. The default is to convert everything.
 *
 * @return string - Returns the encoded string.
 *
 */
<<__Native>>
function htmlentities(string $str,
                      int $quote_style = ENT_COMPAT,
                      string $charset = "UTF-8",
                      bool $double_encode = true)[]: string;

/**
 * This function is the opposite of htmlspecialchars(). It converts special
 *   HTML entities back to characters.  The converted entities are: &amp;,
 *   &quot; (when ENT_NOQUOTES is not set), &#039; (when ENT_QUOTES is set),
 *   &lt; and &gt;.
 *
 * @param string $str - The string to decode
 * @param int $quote_style - The quote style. One of the following constants:
 *   quote_style constants Constant Name Description ENT_COMPAT Will convert
 *   double-quotes and leave single-quotes alone (default) ENT_QUOTES Will
 *   convert both double and single quotes ENT_NOQUOTES Will leave both double
 *   and single quotes unconverted
 *
 * @return string - Returns the decoded string.
 *
 */
<<__Native>>
function htmlspecialchars_decode(string $str,
                                 int $quote_style = ENT_COMPAT)[]: string;

/**
 * Certain characters have special significance in HTML, and should be
 *   represented by HTML entities if they are to preserve their meanings. This
 *   function returns a string with some of these conversions made; the
 *   translations made are those most useful for everyday web programming. If
 *   you require all HTML character entities to be translated, use
 *   htmlentities() instead. This function is useful in preventing user-supplied
 *   text from containing HTML markup, such as in a message board or guest book
 *   application.  The translations performed are: '&' (ampersand) becomes
 *   '&amp;' '"' (double quote) becomes '&quot;' when ENT_NOQUOTES is not set.
 *   ''' (single quote) becomes '&#039;' only when ENT_QUOTES is set. '<' (less
 *   than) becomes '&lt;' '>' (greater than) becomes '&gt;'
 *
 * @param string $str - The string being converted.
 * @param int $quote_style - The optional second argument, quote_style, tells
 *   the function what to do with single and double quote characters. The
 *   default mode, ENT_COMPAT, is the backwards compatible mode which only
 *   translates the double-quote character and leaves the single-quote
 *   untranslated. If ENT_QUOTES is set, both single and double quotes are
 *   translated and if ENT_NOQUOTES is set neither single nor double quotes are
 *   translated.
 * @param string $charset - Defines character set used in conversion. The
 *   default character set is ISO-8859-1.  For the purposes of this function,
 *   the charsets ISO-8859-1, ISO-8859-15, UTF-8, cp866, cp1251, cp1252, and
 *   KOI8-R are effectively equivalent, as the characters affected by
 *   htmlspecialchars() occupy the same positions in all of these charsets.
 *   Following character sets are supported in PHP 4.3.0 and later. Supported
 *   charsets Charset Aliases Description ISO-8859-1 ISO8859-1 Western European,
 *   Latin-1 ISO-8859-15 ISO8859-15 Western European, Latin-9. Adds the Euro
 *   sign, French and Finnish letters missing in Latin-1(ISO-8859-1). UTF-8
 *   ASCII compatible multi-byte 8-bit Unicode. cp866 ibm866, 866 DOS-specific
 *   Cyrillic charset. This charset is supported in 4.3.2. cp1251 Windows-1251,
 *   win-1251, 1251 Windows-specific Cyrillic charset. This charset is supported
 *   in 4.3.2. cp1252 Windows-1252, 1252 Windows specific charset for Western
 *   European. KOI8-R koi8-ru, koi8r Russian. This charset is supported in
 *   4.3.2. BIG5 950 Traditional Chinese, mainly used in Taiwan. GB2312 936
 *   Simplified Chinese, national standard character set. BIG5-HKSCS  Big5 with
 *   Hong Kong extensions, Traditional Chinese. Shift_JIS SJIS, 932 Japanese
 *   EUC-JP EUCJP Japanese Any other character sets are not recognized and
 *   ISO-8859-1 will be used instead.
 * @param bool $double_encode - When double_encode is turned off PHP will not
 *   encode existing html entities, the default is to convert everything.
 *
 * @return string - The converted string.
 *
 */
<<__Native>>
function htmlspecialchars(string $str,
                          int $quote_style = ENT_COMPAT,
                          string $charset = "UTF-8",
                          bool $double_encode = true)[]: string;

/**
 * Certain characters have special significance in HTML, and should be
 *   represented by HTML entities if they are to preserve their meanings. This
 *   function returns a string with some of these conversions made; the
 *   translations made are those most useful for everyday web programming. If
 *   you require all HTML character entities to be translated, use
 *   htmlentities() instead. This function is useful in preventing user-supplied
 *   text from containing HTML markup, such as in a message board or guest book
 *   application.  The translations performed are: '&' (ampersand) becomes
 *   '&amp;' '"' (double quote) becomes '&quot;' when ENT_NOQUOTES is not set.
 *   ''' (single quote) becomes '&#039;' only when ENT_QUOTES is set. '<' (less
 *   than) becomes '&lt;' '>' (greater than) becomes '&gt;'
 *
 * @param string $str - The string being converted.
 * @param int $quote_style - The optional second argument, quote_style, tells
 *   the function what to do with single and double quote characters. The
 *   default mode, ENT_COMPAT, is the backwards compatible mode which only
 *   translates the double-quote character and leaves the single-quote
 *   untranslated. If ENT_QUOTES is set, both single and double quotes are
 *   translated and if ENT_NOQUOTES is set neither single nor double quotes are
 *   translated.
 * @param string $charset - Defines character set used in conversion. The
 *   default character set is ISO-8859-1.  For the purposes of this function,
 *   the charsets ISO-8859-1, ISO-8859-15, UTF-8, cp866, cp1251, cp1252, and
 *   KOI8-R are effectively equivalent, as the characters affected by
 *   htmlspecialchars() occupy the same positions in all of these charsets.
 *   Following character sets are supported in PHP 4.3.0 and later. Supported
 *   charsets Charset Aliases Description ISO-8859-1 ISO8859-1 Western European,
 *   Latin-1 ISO-8859-15 ISO8859-15 Western European, Latin-9. Adds the Euro
 *   sign, French and Finnish letters missing in Latin-1(ISO-8859-1). UTF-8
 *   ASCII compatible multi-byte 8-bit Unicode. cp866 ibm866, 866 DOS-specific
 *   Cyrillic charset. This charset is supported in 4.3.2. cp1251 Windows-1251,
 *   win-1251, 1251 Windows-specific Cyrillic charset. This charset is supported
 *   in 4.3.2. cp1252 Windows-1252, 1252 Windows specific charset for Western
 *   European. KOI8-R koi8-ru, koi8r Russian. This charset is supported in
 *   4.3.2. BIG5 950 Traditional Chinese, mainly used in Taiwan. GB2312 936
 *   Simplified Chinese, national standard character set. BIG5-HKSCS  Big5 with
 *   Hong Kong extensions, Traditional Chinese. Shift_JIS SJIS, 932 Japanese
 *   EUC-JP EUCJP Japanese Any other character sets are not recognized and
 *   ISO-8859-1 will be used instead.
 * @param array $extra - An array of extra ascii chars to be encoded.
 *
 * @return string - The converted string.
 *
 */
<<__Native>>
function fb_htmlspecialchars(string $str,
                             int $quote_style = ENT_COMPAT,
                             string $charset = "ISO-8859-1",
                             mixed $extra = varray[]): string;

/**
 * Returns a quoted printable string created according to  RFC2045, section
 *   6.7.  This function is similar to imap_8bit(), except this one does not
 *   require the IMAP module to work.
 *
 * @param string $str - The input string.
 *
 * @return string - Returns the encoded string.
 *
 */
<<__IsFoldable, __Native>>
function quoted_printable_encode(string $str)[]: string;

/**
 * This function returns an 8-bit binary string corresponding to the decoded
 *   quoted printable string (according to  RFC2045, section 6.7, not  RFC2821,
 *   section 4.5.2, so additional periods are not stripped from the beginning of
 *   line).  This function is similar to imap_qprint(), except this one does not
 *   require the IMAP module to work.
 *
 * @param string $str - The input string.
 *
 * @return string - Returns the 8-bit binary string.
 *
 */
<<__IsFoldable, __Native>>
function quoted_printable_decode(string $str)[]: string;

/**
 * convert_uudecode() decodes a uuencoded string.
 *
 * @param string $data - The uuencoded data.
 *
 * @return mixed - Returns the decoded data as a string.
 *
 */
<<__IsFoldable, __Native>>
function convert_uudecode(string $data)[]: mixed;

/**
 * convert_uuencode() encodes a string using the uuencode algorithm.  Uuencode
 *   translates all strings (including binary's ones) into printable characters,
 *   making them safe for network transmissions. Uuencoded data is about 35%
 *   larger than the original.
 *
 * @param string $data - The data to be encoded.
 *
 * @return mixed - Returns the uuencoded data.
 *
 */
<<__IsFoldable, __Native>>
function convert_uuencode(string $data)[]: mixed;

/**
 * Performs the ROT13 encoding on the str argument and returns the resulting
 *   string.  The ROT13 encoding simply shifts every letter by 13 places in the
 *   alphabet while leaving non-alpha characters untouched. Encoding and
 *   decoding are done by the same function, passing an encoded string as
 *   argument will return the original version.
 *
 * @param string $str - The input string.
 *
 * @return string - Returns the ROT13 version of the given string.
 *
 */
<<__IsFoldable, __Native>>
function str_rot13(string $str)[]: string;

/**
 * Generates the cyclic redundancy checksum polynomial of 32-bit lengths of
 *   the str. This is usually used to validate the integrity of data being
 *   transmitted.  Because PHP's integer type is signed, and many crc32
 *   checksums will result in negative integers, you need to use the "%u"
 *   formatter of sprintf() or printf() to get the string representation of the
 *   unsigned crc32 checksum.
 *
 * @param string $str - The data.
 *
 * @return int - Returns the crc32 checksum of str as an integer.
 *
 */
<<__IsFoldable, __Native>>
function crc32(string $str)[]: int;

/**
 * crypt() will return a hashed string using the standard Unix DES-based
 *   algorithm or alternative algorithms that may be available on the system.
 *   Some operating systems support more than one type of hash. In fact,
 *   sometimes the standard DES-based algorithm is replaced by an MD5-based
 *   algorithm. The hash type is triggered by the salt argument. Prior to 5.3,
 *   PHP would determine the available algorithms at install-time based on the
 *   system's crypt(). If no salt is provided, PHP will auto-generate either a
 *   standard two character (DES) salt, or a twelve character (MD5), depending
 *   on the availability of MD5 crypt(). PHP sets a constant named
 *   CRYPT_SALT_LENGTH which indicates the longest valid salt allowed by the
 *   available hashes.  The standard DES-based crypt() returns the salt as the
 *   first two characters of the output. It also only uses the first eight
 *   characters of str, so longer strings that start with the same eight
 *   characters will generate the same result (when the same salt is used). On
 *   systems where the crypt() function supports multiple hash types, the
 *   following constants are set to 0 or 1 depending on whether the given type
 *   is available: CRYPT_STD_DES - Standard DES-based hash with a two character
 *   salt from the alphabet "./0-9A-Za-z". Using invalid characters in the salt
 *   will cause crypt() to fail. CRYPT_EXT_DES - Extended DES-based hash. The
 *   "salt" is a 9-character string consisting of an underscore followed by 4
 *   bytes of iteration count and 4 bytes of salt. These are encoded as
 *   printable characters, 6 bits per character, least significant character
 *   first. The values 0 to 63 are encoded as "./0-9A-Za-z". Using invalid
 *   characters in the salt will cause crypt() to fail. CRYPT_MD5 - MD5 hashing
 *   with a twelve character salt starting with $1$ CRYPT_BLOWFISH - Blowfish
 *   hashing with a salt as follows: "$2a$", a two digit cost parameter, "$",
 *   and 22 base 64 digits from the alphabet "./0-9A-Za-z". Using characters
 *   outside of this range in the salt will cause crypt() to return a
 *   zero-length string. The two digit cost parameter is the base-2 logarithm of
 *   the iteration count for the underlying Blowfish-based hashing algorithmeter
 *   and must be in range 04-31, values outside this range will cause crypt() to
 *   fail. CRYPT_SHA256 - SHA-256 hash with a sixteen character salt prefixed
 *   with $5$. If the salt string starts with 'rounds=<N>$', the numeric value
 *   of N is used to indicate how many times the hashing loop should be
 *   executed, much like the cost parameter on Blowfish. The default number of
 *   rounds is 5000, there is a minimum of 1000 and a maximum of 999,999,999.
 *   Any selection of N outside this range will be truncated to the nearest
 *   limit. CRYPT_SHA512 - SHA-512 hash with a sixteen character salt prefixed
 *   with $6$. If the salt string starts with 'rounds=<N>$', the numeric value
 *   of N is used to indicate how many times the hashing loop should be
 *   executed, much like the cost parameter on Blowfish. The default number of
 *   rounds is 5000, there is a minimum of 1000 and a maximum of 999,999,999.
 *   Any selection of N outside this range will be truncated to the nearest
 *   limit.  As of PHP 5.3.0, PHP contains its own implementation and will use
 *   that if the system lacks of support for one or more of the algorithms.
 *
 * @param string $str - The string to be hashed.
 * @param string $salt - An optional salt string to base the hashing on. If
 *   not provided, the behaviour is defined by the algorithm implementation and
 *   can lead to unexpected results.
 *
 * @return string - Returns the hashed string or a string that is shorter than
 *   13 characters and is guaranteed to differ from the salt on failure.
 *
 */
<<__Native>>
function crypt(string $str, string $salt = ""): string;

/**
 * Calculates the MD5 hash of str using the RSA Data Security, Inc. MD5
 *   Message-Digest Algorithm, and returns that hash.
 *
 * @param string $str - The string.
 * @param bool $raw_output - If the optional raw_output is set to TRUE, then
 *   the md5 digest is instead returned in raw binary format with a length of
 *   16.
 *
 * @return string - Returns the hash as a 32-character hexadecimal number.
 *
 */
<<__IsFoldable, __Native>>
function md5(string $str, bool $raw_output = false)[]: string;

/**
 * @param string $str - The input string.
 * @param bool $raw_output - If the optional raw_output is set to TRUE, then
 *   the sha1 digest is instead returned in raw binary format with a length of
 *   20, otherwise the returned value is a 40-character hexadecimal number.
 *
 * @return string - Returns the sha1 hash as a string.
 *
 */
<<__IsFoldable, __Native>>
function sha1(string $str, bool $raw_output = false)[]: string;

/**
 * This function returns a copy of str, translating all occurrences of each
 *   character in from to the corresponding character in to.  If from and to are
 *   different lengths, the extra characters in the longer of the two are
 *   ignored.
 *
 * @param string $str - The string being translated.
 * @param mixed $from - The string being translated to to.
 * @param mixed $to - The string replacing from.
 *
 * @return mixed - Returns the translated string.  If replace_pairs contains a
 *   key which is an empty string (""), FALSE will be returned.
 *
 */
<<__IsFoldable, __Native>>
function strtr(string $str, mixed $from, mixed $to = null)[]: mixed;

/**
 * Converts from one Cyrillic character set to another. Supported characters
 *   are: k - koi8-r w - windows-1251 i - iso8859-5 a - x-cp866 d - x-cp866 m -
 *   x-mac-cyrillic
 *
 * @param string $str - The string to be converted.
 * @param string $from - The source Cyrillic character set, as a single
 *   character.
 * @param string $to - The target Cyrillic character set, as a single
 *   character.
 *
 * @return string - Returns the converted string.
 *
 */
<<__IsFoldable, __Native>>
function convert_cyr_string(string $str, string $from, string $to)[]: string;

/**
 * get_html_translation_table() will return the translation table that is used
 *   internally for htmlspecialchars() and htmlentities() with the default
 *   charset.  Special characters can be encoded in several ways. E.g. " can be
 *   encoded as &quot;, &#34; or &#x22. get_html_translation_table() returns
 *   only the most common form for them.
 *
 * @param int $table - There are two new constants (HTML_ENTITIES,
 *   HTML_SPECIALCHARS) that allow you to specify the table you want.
 * @param int $quote_style - Like the htmlspecialchars() and htmlentities()
 *   functions you can optionally specify the quote_style you are working with.
 *   See the description of these modes in htmlspecialchars().
 * @param string $encoding - Encoding to use. If omitted, the default value
 *   for this argument is ISO-8859-1 in versions of PHP prior to 5.4.0, and
 *   UTF-8 from PHP 5.4.0 onwards.
 *
 * @return array - Returns the translation table as an array.
 *
 */
<<__Native>>
function get_html_translation_table(
  int $table = 0,
  int $quote_style = ENT_COMPAT,
  string $encoding = "UTF-8",
)[]: darray<string, string>;
// TODO(T120001721) This type is nullable, specifically a
// `?darray<string, string>`; HHVM's native interface doesn't allow us to
// differentiate between nullable and non-nullable strings, arrays, or objects.

/**
 * Converts logical Hebrew text to visual text.  The function tries to avoid
 *   breaking words.
 *
 * @param string $hebrew_text - A Hebrew input string.
 * @param int $max_chars_per_line - This optional parameter indicates maximum
 *   number of characters per line that will be returned.
 *
 * @return string - Returns the visual string.
 *
 */
<<__Native>>
function hebrev(string $hebrew_text, int $max_chars_per_line = 0): string;

/**
 * This function is similar to hebrev() with the difference that it converts
 *   newlines (\n) to "<br>\n".  The function tries to avoid breaking words.
 *
 * @param string $hebrew_text - A Hebrew input string.
 *
 * @param int $max_chars_per_line - This optional parameter indicates maximum
 *   number of characters per line that will be returned.
 *
 * @return string - Returns the visual string.
 *
 */
<<__Native>>
function hebrevc(string $hebrew_text, int $max_chars_per_line = 0): string;

/**
 * Sets locale information. On Windows, setlocale(LC_ALL, '') sets the locale
 *   names from the system's regional/language settings (accessible via Control
 *   Panel).
 *
 * @param int $category - category is a named constant specifying the category
 *   of the functions affected by the locale setting: LC_ALL for all of the
 *   below LC_COLLATE for string comparison, see strcoll() LC_CTYPE for
 *   character classification and conversion, for example strtoupper()
 *   LC_MONETARY for localeconv() LC_NUMERIC for decimal separator (See also
 *   localeconv()) LC_TIME for date and time formatting with strftime()
 *   LC_MESSAGES for system responses (available if PHP was compiled with
 *   libintl)
 * @param mixed $locale - If locale is NULL or the empty string "", the locale
 *   names will be set from the values of environment variables with the same
 *   names as the above categories, or from "LANG".  If locale is "0", the
 *   locale setting is not affected, only the current setting is returned.  If
 *   locale is an array or followed by additional parameters then each array
 *   element or parameter is tried to be set as new locale until success. This
 *   is useful if a locale is known under different names on different systems
 *   or for providing a fallback for a possibly not available locale.
 *
 * @return mixed - Returns the new current locale, or FALSE if the locale
 *   functionality is not implemented on your platform, the specified locale
 *   does not exist or the category name is invalid.  An invalid category name
 *   also causes a warning message. Category/locale names can be found in  RFC
 *   1766 and  ISO 639. Different systems have different naming schemes for
 *   locales.  The return value of setlocale() depends on the system that PHP is
 *   running. It returns exactly what the system setlocale function returns.
 *
 */
<<__Native>>
function setlocale(int $category, mixed $locale, mixed... $argv): mixed;

/**
 * Returns an associative array containing localized numeric and monetary
 *   formatting information.
 *
 * @return array
 *
 */
<<__Native>>
function localeconv(): darray<string, mixed>;
// TODO(T119996979) `localeconv` returns a shape, not a darray

/**
 * nl_langinfo() is used to access individual elements of the locale
 *   categories. Unlike localeconv(), which returns all of the elements,
 *   nl_langinfo() allows you to select any specific element.
 *
 * @param int $item
 *
 * @return string - Returns the element as a string, or FALSE if item is not
 *   valid.
 *
 */
<<__Native>>
function nl_langinfo(int $item): mixed;

/**
 * The function sscanf() is the input analog of printf(). sscanf() reads from
 *   the string str and interprets it according to the specified format, which
 *   is described in the documentation for sprintf().  Any whitespace in the
 *   format string matches any whitespace in the input string. This means that
 *   even a tab \t in the format string can match a single space character in
 *   the input string.
 *
 * @param string $str - The input string being parsed.
 * @param string $format - The interpreted format for str, which is described
 *   in the documentation for sprintf() with following differences: Function is
 *   not locale-aware. F, g, G and b are not supported. D stands for decimal
 *   number. i stands for integer with base detection. n stands for number of
 *   characters processed so far.
 *
 * @return mixed - If only two parameters were passed to this function, the
 *   values parsed will be returned as an array. Otherwise, if optional
 *   parameters are passed, the function will return the number of assigned
 *   values. The optional parameters must be passed by reference.
 *
 */
<<__Native>>
function sscanf(string $str, string $format)[]: mixed;

/**
 * Returns a one-character string containing the character specified by ascii.
 *    This function complements ord().
 *
 * @param int $ascii - The ascii code.
 *
 * @return string - Returns the specified character.
 *
 */
<<__IsFoldable, __Native>>
function chr(mixed $ascii)[]: string;

/**
 * Returns the ASCII value of the first character of string.  This function
 *   complements chr().
 *
 * @param string $str - A character.
 *
 * @return int - Returns the ASCII value as an integer.
 *
 */
<<__IsFoldable, __Native>>
function ord(string $str)[]: int;

/**
 * money_format() returns a formatted version of number. This function wraps
 *   the C library function strfmon(), with the difference that this
 *   implementation converts only one number at a time.
 *
 * @param string $format - The format specification consists of the following
 *   sequence: % character Flags  One or more of the optional flags below can be
 *   used: =f  The character = followed by a (single byte) character f to be
 *   used as the numeric fill character. The default fill character is space.
 * @param float $number - Disable the use of grouping characters (as defined
 *   by the current locale).
 *
 * @return mixed - Returns the formatted string. Characters before and after
 *   the formatting string will be returned unchanged. Non-numeric number causes
 *   returning NULL and emitting E_WARNING.
 *
 */
<<__Native>>
function money_format(string $format, float $number): mixed;

/**
 * This function accepts either one, two, or four parameters (not three):  If
 *   only one parameter is given, number will be formatted without decimals, but
 *   with a comma (",") between every group of thousands.  If two parameters are
 *   given, number will be formatted with decimals decimals with a dot (".") in
 *   front, and a comma (",") between every group of thousands.  If all four
 *   parameters are given, number will be formatted with decimals decimals,
 *   dec_point instead of a dot (".") before the decimals and thousands_sep
 *   instead of a comma (",") between every group of thousands.
 *
 * @param float $number - The number being formatted.
 * @param int $decimals - Sets the number of decimal points.
 * @param mixed $dec_point - Sets the separator for the decimal point.
 * @param mixed $thousands_sep - Sets the thousands separator.  Only the first
 *   character of thousands_sep is used. For example, if you use bar as
 *   thousands_sep on the number 1000, number_format() will return 1b000.
 *
 * @return string - A formatted version of number.
 *
 */
<<__Native>>
function number_format(float $number,
                       int $decimals = 0,
                       mixed $dec_point = ".",
                       mixed $thousands_sep = ",")[]: string;

/**
 * @param string $str1 - The first string.
 * @param string $str2 - The second string.
 *
 * @return int - Returns < 0 if str1 is less than str2; > 0 if str1 is greater
 *   than str2, and 0 if they are equal.
 *
 */
<<__IsFoldable, __Native>>
function strcmp(string $str1, string $str2)[]: int;

/**
 * This function is similar to strcmp(), with the difference that you can
 *   specify the (upper limit of the) number of characters from each string to
 *   be used in the comparison.  Note that this comparison is case sensitive.
 *
 * @param string $str1 - The first string.
 * @param string $str2 - The second string.
 * @param int $len - Number of characters to use in the comparison.
 *
 * @return mixed - Returns < 0 if str1 is less than str2; > 0 if str1 is
 *   greater than str2, and 0 if they are equal.
 *
 */
<<__IsFoldable, __Native>>
function strncmp(string $str1, string $str2, int $len)[]: mixed;

/**
 * This function implements a comparison algorithm that orders alphanumeric
 *   strings in the way a human being would, this is described as a "natural
 *   ordering". Note that this comparison is case sensitive.
 *
 * @param string $str1 - The first string.
 * @param string $str2 - The second string.
 *
 * @return int - Similar to other string comparison functions, this one
 *   returns < 0 if str1 is less than str2; > 0 if str1 is greater than str2,
 *   and 0 if they are equal.
 *
 */
<<__IsFoldable, __Native>>
function strnatcmp(string $str1, string $str2)[]: int;

/**
 * Binary safe case-insensitive string comparison.
 *
 * @param string $str1 - The first string
 * @param string $str2 - The second string
 *
 * @return int - Returns < 0 if str1 is less than str2; > 0 if str1 is greater
 *   than str2, and 0 if they are equal.
 *
 */
<<__IsFoldable, __Native>>
function strcasecmp(string $str1, string $str2)[]: int;

/**
 * This function is similar to strcasecmp(), with the difference that you can
 *   specify the (upper limit of the) number of characters from each string to
 *   be used in the comparison.
 *
 * @param string $str1 - The first string.
 * @param string $str2 - The second string.
 * @param int $len - The length of strings to be used in the comparison.
 *
 * @return mixed - Returns < 0 if str1 is less than str2; > 0 if str1 is
 *   greater than str2, and 0 if they are equal.
 *
 */
<<__IsFoldable, __Native>>
function strncasecmp(string $str1, string $str2, int $len)[]: mixed;

/**
 * This function implements a comparison algorithm that orders alphanumeric
 *   strings in the way a human being would. The behaviour of this function is
 *   similar to strnatcmp(), except that the comparison is not case sensitive.
 *   For more information see: Martin Pool's  Natural Order String Comparison
 *   page.
 *
 * @param string $str1 - The first string.
 * @param string $str2 - The second string.
 *
 * @return int - Similar to other string comparison functions, this one
 *   returns < 0 if str1 is less than str2 > 0 if str1 is greater than str2, and
 *   0 if they are equal.
 *
 */
<<__IsFoldable, __Native>>
function strnatcasecmp(string $str1, string $str2)[]: int;

/**
 * Note that this comparison is case sensitive, and unlike strcmp() this
 *   function is not binary safe.  strcoll() uses the current locale for doing
 *   the comparisons. If the current locale is C or POSIX, this function is
 *   equivalent to strcmp().
 *
 * @param string $str1 - The first string.
 * @param string $str2 - The second string.
 *
 * @return int - Returns < 0 if str1 is less than str2; > 0 if str1 is greater
 *   than str2, and 0 if they are equal.
 *
 */
<<__IsFoldable, __Native>>
function strcoll(string $str1, string $str2)[]: int;

/**
 * substr_compare() compares main_str from position offset with str up to
 *   length characters.
 *
 * @param string $main_str - The main string being compared.
 * @param string $str - The secondary string being compared.
 * @param int $offset - The start position for the comparison. If negative, it
 *   starts counting from the end of the string.
 * @param int $length - The length of the comparison. The default value is the
 *   largest of the length of the str compared to the length of main_str less
 *   the offset.
 * @param bool $case_insensitivity - If case_insensitivity is TRUE, comparison
 *   is case insensitive.
 *
 * @return mixed - Returns < 0 if main_str from position offset is less than
 *   str, > 0 if it is greater than str, and 0 if they are equal. If offset is
 *   equal to or greater than the length of main_str or length is set and is
 *   less than 1, substr_compare() prints a warning and returns FALSE.
 *
 */
<<__IsFoldable, __Native>>
function substr_compare(string $main_str,
                        string $str,
                        int $offset,
                        int $length = 0x7FFFFFFF,
                        bool $case_insensitivity = false)[]: mixed;

<<__IsFoldable, __Native>>
function strchr(string $haystack, mixed $needle)[]: mixed;

/**
 * This function returns the portion of haystack which starts at the last
 *   occurrence of needle and goes until the end of haystack.
 *
 * @param string $haystack - The string to search in
 * @param mixed $needle - If needle contains more than one character, only the
 *   first is used. This behavior is different from that of strstr().  If needle
 *   is not a string, it is converted to an integer and applied as the ordinal
 *   value of a character.
 *
 * @return mixed - This function returns the portion of string, or FALSE if
 *   needle is not found.
 *
 */
<<__IsFoldable, __Native>>
function strrchr(string $haystack, mixed $needle)[]: mixed;

/**
 * Returns part of haystack string from the first occurrence of needle to the
 *   end of haystack.  This function is case-sensitive. For case-insensitive
 *   searches, use stristr().  If you only want to determine if a particular
 *   needle occurs within haystack, use the faster and less memory intensive
 *   function strpos() instead.
 *
 * @param string $haystack - The input string.
 * @param mixed $needle - If needle is not a string, it is converted to an
 *   integer and applied as the ordinal value of a character.
 * @param bool $before_needle - If TRUE, strstr() returns the part of the
 *   haystack before the first occurrence of the needle (excluding the needle).
 *
 * @return mixed - Returns the portion of string, or FALSE if needle is not
 *   found.
 *
 */
<<__IsFoldable, __Native>>
function strstr(string $haystack,
                mixed $needle,
                bool $before_needle = false)[]: mixed;

/**
 * Returns all of haystack from the first occurrence of needle to the end.
 *   needle and haystack are examined in a case-insensitive manner.
 *
 * @param string $haystack - The string to search in
 * @param mixed $needle - If needle is not a string, it is converted to an
 *   integer and applied as the ordinal value of a character.
 * @param bool $before_needle - If TRUE, strstr() returns the part of the
 *   haystack before the first occurrence of the needle (excluding the needle).
 *
 * @return mixed - Returns the matched substring. If needle is not found,
 *   returns FALSE.
 *
 */
<<__IsFoldable, __Native>>
function stristr(string $haystack,
                 mixed $needle,
                 bool $before_needle = false)[]: mixed;

/**
 * strpbrk() searches the haystack string for a char_list.
 *
 * @param string $haystack - The string where char_list is looked for.
 * @param string $char_list - This parameter is case sensitive.
 *
 * @return mixed - Returns a string starting from the character found, or
 *   FALSE if it is not found.
 *
 */
<<__IsFoldable, __Native>>
function strpbrk(string $haystack, string $char_list)[]: mixed;

/**
 * Returns the numeric position of the first occurrence of needle in the
 *   haystack string. Unlike the strrpos() before PHP 5, this function can take
 *   a full string as the needle parameter and the entire string will be used.
 *
 * @param string $haystack - The string to search in
 * @param mixed $needle - If needle is not a string, it is converted to an
 *   integer and applied as the ordinal value of a character.
 * @param int $offset - The optional offset parameter allows you to specify
 *   which character in haystack to start searching. The position returned is
 *   still relative to the beginning of haystack.
 *
 * @return mixed - Returns the position as an integer. If needle is not found,
 *   strpos() will return boolean FALSE. WarningThis function may return Boolean
 *   FALSE, but may also return a non-Boolean value which evaluates to FALSE,
 *   such as 0 or "". Please read the section on Booleans for more information.
 *   Use the === operator for testing the return value of this function.
 *
 */
<<__IsFoldable, __Native>>
function strpos(string $haystack, mixed $needle, int $offset = 0)[]: mixed;

/**
 * Returns the numeric position of the first occurrence of needle in the
 *   haystack string.  Unlike strpos(), stripos() is case-insensitive.
 *
 * @param string $haystack - The string to search in
 * @param mixed $needle - Note that the needle may be a string of one or more
 *   characters.  If needle is not a string, it is converted to an integer and
 *   applied as the ordinal value of a character.
 * @param int $offset - The optional offset parameter allows you to specify
 *   which character in haystack to start searching. The position returned is
 *   still relative to the beginning of haystack.
 *
 * @return mixed - If needle is not found, stripos() will return boolean
 *   FALSE. WarningThis function may return Boolean FALSE, but may also return a
 *   non-Boolean value which evaluates to FALSE, such as 0 or "". Please read
 *   the section on Booleans for more information. Use the === operator for
 *   testing the return value of this function.
 *
 */
<<__IsFoldable, __Native>>
function stripos(string $haystack, mixed $needle, int $offset = 0)[]: mixed;

/**
 * Returns the numeric position of the last occurrence of needle in the
 *   haystack string. Note that the needle in this case can only be a single
 *   character in PHP 4. If a string is passed as the needle, then only the
 *   first character of that string will be used.
 *
 * @param string $haystack - The string to search in.
 * @param mixed $needle - If needle is not a string, it is converted to an
 *   integer and applied as the ordinal value of a character.
 * @param int $offset - May be specified to begin searching an arbitrary
 *   number of characters into the string. Negative values will stop searching
 *   at an arbitrary point prior to the end of the string.
 *
 * @return mixed - Returns the position where the needle exists. Returns FALSE
 *   if the needle was not found.
 *
 */
<<__IsFoldable, __Native>>
function strrpos(string $haystack, mixed $needle, int $offset = 0)[]: mixed;

/**
 * Find position of last occurrence of a case-insensitive string in a string.
 *   Unlike strrpos(), strripos() is case-insensitive.
 *
 * @param string $haystack - The string to search in
 * @param mixed $needle - Note that the needle may be a string of one or more
 *   characters.
 * @param int $offset - The offset parameter may be specified to begin
 *   searching an arbitrary number of characters into the string.  Negative
 *   offset values will start the search at offset characters from the start of
 *   the string.
 *
 * @return mixed - Returns the numerical position of the last occurrence of
 *   needle. Also note that string positions start at 0, and not 1.  If needle
 *   is not found, FALSE is returned. WarningThis function may return Boolean
 *   FALSE, but may also return a non-Boolean value which evaluates to FALSE,
 *   such as 0 or "". Please read the section on Booleans for more information.
 *   Use the === operator for testing the return value of this function.
 *
 */
<<__IsFoldable, __Native>>
function strripos(string $haystack, mixed $needle, int $offset = 0)[]: mixed;

/**
 * substr_count() returns the number of times the needle substring occurs in
 *   the haystack string. Please note that needle is case sensitive.  This
 *   function doesn't count overlapped substrings. See the example below!
 *
 * @param string $haystack - The string to search in
 * @param string $needle - The substring to search for
 * @param int $offset - The offset where to start counting
 * @param int $length - The maximum length after the specified offset to
 *   search for the substring. It outputs a warning if the offset plus the
 *   length is greater than the haystack length.
 *
 * @return mixed - This functions returns an integer.
 *
 */
<<__IsFoldable, __Native>>
function substr_count(string $haystack,
                      string $needle,
                      int $offset = 0,
                      int $length = 0x7FFFFFFF)[]: mixed;

/**
 * Returns the length of the first group of consecutive characters from mask
 *   found in subject.  If start and length are omitted, then all of subject
 *   will be examined. If they are included, then the effect will be the same as
 *   calling strspn(substr($subject, $start, $length), $mask) (see substr for
 *   more information).  The line of code:  will assign 2 to $var, because the
 *   string "42" is the first segment from subject to consist only of characters
 *   contained within "1234567890".
 *
 * @param string $str1 - The string to examine.
 * @param string $str2 - The list of allowable characters to include in
 *   counted segments.
 * @param int $start - The position in subject to start searching.  If start
 *   is given and is non-negative, then strspn() will begin examining subject at
 *   the start'th position. For instance, in the string 'abcdef', the character
 *   at position 0 is 'a', the character at position 2 is 'c', and so forth.  If
 *   start is given and is negative, then strspn() will begin examining subject
 *   at the start'th position from the end of subject.
 * @param int $length - The length of the segment from subject to examine.  If
 *   length is given and is non-negative, then subject will be examined for
 *   length characters after the starting position.  If lengthis given and is
 *   negative, then subject will be examined from the starting position up to
 *   length characters from the end of subject.
 *
 * @return mixed - Returns the length of the initial segment of str1 which
 *   consists entirely of characters in str2.
 *
 */
<<__IsFoldable, __Native>>
function strspn(string $str1,
                string $str2,
                int $start = 0,
                int $length = 0x7FFFFFFF)[]: mixed;

/**
 * Returns the length of the initial segment of str1 which does not contain
 *   any of the characters in str2.
 *
 * @param string $str1 - The first string.
 * @param string $str2 - The second string.
 * @param int $start - The start position of the string to examine.
 * @param int $length - The length of the string to examine.
 *
 * @return mixed - Returns the length of the segment as an integer.
 *
 */
<<__IsFoldable, __Native>>
function strcspn(string $str1,
                 string $str2,
                 int $start = 0,
                 int $length = 0x7FFFFFFF)[]: mixed;

/**
 * Returns the length of the given string.
 *
 * @param mixed $vstr - The string being measured for length.
 *
 * @return mixed - The length of the string on success, and 0 if the string is
 *   empty.
 *
 */
<<__IsFoldable, __Native>>
function strlen(string $vstr)[]: int;

/**
 * Parses a string input for fields in CSV format and returns an array
 *   containing the fields read.
 *
 * @param string $input - The string to parse.
 * @param string $delimiter - Set the field delimiter (one character only).
 * @param string $enclosure - Set the field enclosure character (one character
 *   only).
 * @param string $escape - Set the escape character (one character only).
 *   Defaults as a backslash (\)
 *
 * @return array - Returns an indexed array containing the fields read.
 *
 */
<<__Native>>
function str_getcsv(string $input,
                    string $delimiter = ",",
                    string $enclosure = "\"",
                    string $escape = "\\"): varray<?string>;

/**
 * Counts the number of occurrences of every byte-value (0..255) in string and
 *   returns it in various ways.
 *
 * @param string $str - The examined string.
 * @param int $mode - See return values.
 *
 * @return mixed - Depending on mode count_chars() returns one of the
 *   following: 0 - an array with the byte-value as key and the frequency of
 *   every byte as value. 1 - same as 0 but only byte-values with a frequency
 *   greater than zero are listed. 2 - same as 0 but only byte-values with a
 *   frequency equal to zero are listed. 3 - a string containing all unique
 *   characters is returned. 4 - a string containing all not used characters is
 *   returned.
 *
 */
<<__IsFoldable, __Native>>
function count_chars(string $str, int $mode = 0)[]: mixed;

/**
 * Counts the number of words inside string. If the optional format is not
 *   specified, then the return value will be an integer representing the number
 *   of words found. In the event the format is specified, the return value will
 *   be an array, content of which is dependent on the format. The possible
 *   value for the format and the resultant outputs are listed below.  For the
 *   purpose of this function, 'word' is defined as a locale dependent string
 *   containing alphabetic characters, which also may contain, but not start
 *   with "'" and "-" characters.
 *
 * @param string $str - The string
 * @param int $format - Specify the return value of this function. The current
 *   supported values are: 0 - returns the number of words found 1 - returns an
 *   array containing all the words found inside the string 2 - returns an
 *   associative array, where the key is the numeric position of the word inside
 *   the string and the value is the actual word itself
 * @param string $charlist - A list of additional characters which will be
 *   considered as 'word'
 *
 * @return mixed - Returns an array or an integer, depending on the format
 *   chosen.
 *
 */
<<__IsFoldable, __Native>>
function str_word_count(string $str,
                        int $format = 0,
                        string $charlist = "")[]: mixed;

/**
 * The Levenshtein distance is defined as the minimal number of characters you
 *   have to replace, insert or delete to transform str1 into str2. The
 *   complexity of the algorithm is O(m*n), where n and m are the length of str1
 *   and str2 (rather good when compared to similar_text(), which is
 *   O(max(n,m)**3), but still expensive).  In its simplest form the function
 *   will take only the two strings as parameter and will calculate just the
 *   number of insert, replace and delete operations needed to transform str1
 *   into str2.  A second variant will take three additional parameters that
 *   define the cost of insert, replace and delete operations. This is more
 *   general and adaptive than variant one, but not as efficient.
 *
 * @param string $str1 - One of the strings being evaluated for Levenshtein
 *   distance.
 * @param string $str2 - One of the strings being evaluated for Levenshtein
 *   distance.
 * @param int $cost_ins - Defines the cost of insertion.
 * @param int $cost_rep - Defines the cost of replacement.
 * @param int $cost_del - Defines the cost of deletion.
 *
 * @return int - This function returns the Levenshtein-Distance between the
 *   two argument strings or -1, if one of the argument strings is longer than
 *   the limit of 255 characters.
 *
 */
<<__IsFoldable, __Native>>
function levenshtein(string $str1,
                     string $str2,
                     int $cost_ins = 1,
                     int $cost_rep = 1,
                     int $cost_del = 1)[]: int;

/**
 * This calculates the similarity between two strings as described in Oliver
 *   [1993]. Note that this implementation does not use a stack as in Oliver's
 *   pseudo code, but recursive calls which may or may not speed up the whole
 *   process. Note also that the complexity of this algorithm is O(N**3) where N
 *   is the length of the longest string.
 *
 * @param string $first - The first string.
 * @param string $second - The second string.
 * @param mixed $percent - By passing a reference as third argument,
 *   similar_text() will calculate the similarity in percent for you.
 *
 * @return int - Returns the number of matching chars in both strings.
 *
 */
<<__IsFoldable, __Native>>
function similar_text(string $first,
                      string $second,
                      <<__OutOnly("KindOfDouble")>>
                      inout mixed $percent): int;

/**
 * Calculates the soundex key of str.  Soundex keys have the property that
 *   words pronounced similarly produce the same soundex key, and can thus be
 *   used to simplify searches in databases where you know the pronunciation but
 *   not the spelling. This soundex function returns a string 4 characters long,
 *   starting with a letter.  This particular soundex function is one described
 *   by Donald Knuth in "The Art Of Computer Programming, vol. 3: Sorting And
 *   Searching", Addison-Wesley (1973), pp. 391-392.
 *
 * @param string $str - The input string.
 *
 * @return mixed - Returns the soundex key as a string.
 *
 */
<<__IsFoldable, __Native>>
function soundex(string $str)[]: mixed;

/**
 * Calculates the metaphone key of str.  Similar to soundex() metaphone
 *   creates the same key for similar sounding words. It's more accurate than
 *   soundex() as it knows the basic rules of English pronunciation. The
 *   metaphone generated keys are of variable length.  Metaphone was developed
 *   by Lawrence Philips <lphilips at verity dot com>. It is described in
 *   ["Practical Algorithms for Programmers", Binstock & Rex, Addison Wesley,
 *   1995].
 *
 * @param string $str - The input string.
 * @param int $phones - This parameter restricts the returned metaphone key to
 *   phonemes characters in length. The default value of 0 means no restriction.
 *
 * @return mixed - Returns the metaphone key as a string, or FALSE on failure.
 *
 */
<<__IsFoldable, __Native>>
function metaphone(string $str, int $phones = 0)[]: mixed;

}

namespace HH {

/**
 *
 * @param string $str - The input string
 *
 * @return bool - If $str is "numeric" such that (int)$str would yield either
 * non-zero or 0 for a reason other than that the string didn't look numeric enough
 *
 */
<<__IsFoldable, __Native>>
function str_number_coercible(string $str)[]: bool;

/**
 *
 * @param string $str - The input string
 *
 * @return ?num - If $str is not "numeric" as per the definition of
 * `str_number_coercible` null is returned. Otherwise, return the value of the
 * string coerced to a number
 */
<<__IsFoldable, __Native>>
function str_to_numeric(string $str)[]: ?num;

/**
 * Implements PHP operator ^ (bitwise xor) operator on strings.
 */
<<__IsFoldable, __Native>>
function str_bitwise_xor(string $s1, string $s2)[]: string;

}
