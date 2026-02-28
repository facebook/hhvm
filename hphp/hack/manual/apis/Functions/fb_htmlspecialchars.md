
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Certain characters have special significance in HTML, and should be
represented by HTML entities if they are to preserve their meanings




``` Hack
function fb_htmlspecialchars(
  string $str,
  int $quote_style = ENT_COMPAT,
  string $charset = 'ISO-8859-1',
  array $extra = vec [
],
): string;
```




This
function returns a string with some of these conversions made; the
translations made are those most useful for everyday web programming. If
you require all HTML character entities to be translated, use
htmlentities() instead. This function is useful in preventing user-supplied
text from containing HTML markup, such as in a message board or guest book
application.  The translations performed are: '&' (ampersand) becomes
'&#38;' '"' (double quote) becomes '&#34;' when ENT_NOQUOTES is not set.
''' (single quote) becomes '&#39;' only when ENT_QUOTES is set. '`<`' (less
than) becomes '&#60;' '>' (greater than) becomes '&#62;'




## Parameters




+ ` string $str ` - The string being converted.
+ ` int $quote_style = ENT_COMPAT ` - The optional second argument, quote_style, tells
  the function what to do with single and double quote characters. The
  default mode, ENT_COMPAT, is the backwards compatible mode which only
  translates the double-quote character and leaves the single-quote
  untranslated. If ENT_QUOTES is set, both single and double quotes are
  translated and if ENT_NOQUOTES is set neither single nor double quotes are
  translated.
+ ` string $charset = 'ISO-8859-1' ` - Defines character set used in conversion. The
  default character set is ISO-8859-1.  For the purposes of this function,
  the charsets ISO-8859-1, ISO-8859-15, UTF-8, cp866, cp1251, cp1252, and
  KOI8-R are effectively equivalent, as the characters affected by
  htmlspecialchars() occupy the same positions in all of these charsets.
  Following character sets are supported in PHP 4.3.0 and later. Supported
  charsets Charset Aliases Description ISO-8859-1 ISO8859-1 Western European,
  Latin-1 ISO-8859-15 ISO8859-15 Western European, Latin-9. Adds the Euro
  sign, French and Finnish letters missing in Latin-1(ISO-8859-1). UTF-8
  ASCII compatible multi-byte 8-bit Unicode. cp866 ibm866, 866 DOS-specific
  Cyrillic charset. This charset is supported in 4.3.2. cp1251 Windows-1251,
  win-1251, 1251 Windows-specific Cyrillic charset. This charset is supported
  in 4.3.2. cp1252 Windows-1252, 1252 Windows specific charset for Western
  European. KOI8-R koi8-ru, koi8r Russian. This charset is supported in
  4.3.2. BIG5 950 Traditional Chinese, mainly used in Taiwan. GB2312 936
  Simplified Chinese, national standard character set. BIG5-HKSCS  Big5 with
  Hong Kong extensions, Traditional Chinese. Shift_JIS SJIS, 932 Japanese
  EUC-JP EUCJP Japanese Any other character sets are not recognized and
  ISO-8859-1 will be used instead.
+ ` array $extra = vec [ ] ` - An array of extra ascii chars to be encoded.




## Returns




* ` string ` - - The converted string.
<!-- HHAPIDOC -->
