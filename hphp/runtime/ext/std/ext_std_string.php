<?hh

/* Wraps a string to a given number of characters.
 * @param string $str - The input string.
 * @param int $width - The number of characters at which the string will be
 * wrapped.
 * @param string $break - The line is broken using the optional break
 * parameter.
 * @param bool cut - If the cut is set to true, the string is always wrapped at
 * or before the specified width. So if you have a word that is larger than the
 * given width, it is broken apart.
 * @return mixed - Returns the given string wrapped at the specified length.
 */
<<__Native, __ParamCoerceModeNull>>
function wordwrap(string $str, int $width = 75, string $break = "\n",
                  bool $cut = false): mixed;
