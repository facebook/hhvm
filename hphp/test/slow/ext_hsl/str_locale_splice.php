<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};
use namespace HH\Lib\{Str, Vec};

<<__EntryPoint>>
function main(): void {
  $c = _Locale\get_c_locale();
  $utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);

  $inputs = vec[
    tuple('abcd', 'X', 1, null),
    tuple('abcd', 'X', 1, 1),
    tuple('abcd', 'X', 1, 2),
    tuple('💩bc', 'X', 0, 1),
    tuple('💩bc', 'X', 1, 1),
    tuple('a💩c', 'X', 1, 1),
    tuple('a💩c', 'X', 2, null),
    // These unicode sequences are all é:
    // - `\u{00e9}` is 'é' as a single codepoint
    // - `\u{0065}\u{0301}` is two codepoints - 'e' and a combining accent
    tuple("d\u{00e9}fg", "X", 1, 1),
    tuple("d\u{0065}\u{0301}fg", 'X', 1, 1),
    tuple("d\u{00e9}fg", "X", 2, 1),
    tuple("d\u{0065}\u{0301}fg", 'X', 2, 1),
    tuple("abc", 'X', -1, 1),
    tuple("abc", 'X', -2, 1),
    tuple("💩💩💩", 'X', -1, 1),
    tuple("💩💩💩", 'X', -2, 1),
  ];
  foreach ($inputs as list($str, $replace, $offset, $length)) {
    printf(
      "%s\n(%s, %s, %d, %s)\n\n    Str\\:\t%s\n    C:   \t%s\n    UTF8:\t%s\n",
      Str\repeat('-', 80),
      \var_export($str, true),
      \var_export($replace, true),
      $offset,
      $length === null ? 'null' : (string) $length,
      Str\splice($str, $replace, $offset, $length),
      _Str\splice_l($str, $replace, $offset, $length, $c),
      _Str\splice_l($str, $replace, $offset, $length, $utf8),
    );
  }
}
