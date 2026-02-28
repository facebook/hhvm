
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Cuts a portion of str specified by the start and length parameters




``` Hack
function fb_utf8_substr(
  string $str,
  int $start,
  int $length = PHP_INT_MAX,
): string;
```




## Parameters




+ ` string $str ` - The original string.
+ ` int $start ` - If start is non-negative, fb_utf8_substr() cuts the
  portion out of str beginning at start'th character, counting from zero.  If
  start is negative, fb_utf8_substr() cuts out the portion beginning at the
  position, start characters away from the end of str.
+ ` int $length = PHP_INT_MAX ` - If length is given and is positive, the return value
  will contain at most length characters of the portion that begins at start
  (depending on the length of string).  If negative length is passed,
  fb_utf8_substr() cuts the portion out of str from the start'th character up
  to the character that is length characters away from the end of the string.
  In case start is also negative, the start position is calculated beforehand
  according to the rule explained above.




## Returns




* ` string ` - - Returns the portion of str specified by the start and
  length parameters.  If str is shorter than start characters long, the empty
  string will be returned.
<!-- HHAPIDOC -->
