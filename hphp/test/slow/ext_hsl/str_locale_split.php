<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};
use namespace HH\Lib\{Str, Vec};

function prettify(vec<string> $in): string {
  return Vec\map($in, $it ==> \var_export($it, true))
    |> Str\join($$, ', ');
}

<<__EntryPoint>>
function main(): void {
  $c = _Locale\get_c_locale();
  $utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);

  $inputs = vec[
    tuple('abc', 'a', null),
    tuple('abc', 'b', null),
    tuple('abc', 'c', null),
    tuple('abc', 'd', null),
    tuple('💩bc', 'b', null),
    tuple('a💩b', '💩'[0], null),
    tuple('abc', '', null),
    tuple('a💩b','', null),
    tuple('','', null),
    tuple('','a', null),
    tuple('hello', '', null),
    tuple('hello', '', 3),
    tuple('ababababa', 'b', 3),
    tuple('ababababa', 'b', 30),
    tuple('💩💩💩', '', 2),
    // These unicode sequences are all é:
    // - `\u{00e9}` is 'é' as a single codepoint
    // - `\u{0065}\u{0301}` is two codepoints - 'e' and a combining accent
    tuple("d\u{00e9}f", "\u{00e9}", null),
    tuple("d\u{0065}\u{0301}f", "\u{00e9}", null),
    tuple("d\u{00e9}f", "\u{0065}\u{0301}", null),
    tuple("d\u{0065}\u{0301}f", "\u{0065}\u{0301}", null),
  ];
  foreach ($inputs as list($str, $delim, $limit)) {
    printf(
      "%s\n(%s, %s, %s)\n\n    C:   \t%s\n    UTF8:\t%s\n",
      Str\repeat('-', 80),
      \var_export($str, true),
      \var_export($delim, true),
      $limit === null ? 'null' : (string) $limit,
      prettify(_Str\split_l($str, $delim, $limit, $c)),
      prettify(_Str\split_l($str, $delim, $limit, $utf8)),
    );
  }
}
