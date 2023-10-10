<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {
  $c = _Locale\get_c_locale();
  $en_us_utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);
  $cases = vec[
    tuple('abc', 'a'),
    tuple('abc', 'c'),
    tuple('foo', 'bar'),
    // Differing byte order but still equivalent
    tuple("abc\u{062f}\u{0650}\u{0651}", "\u{062f}\u{0651}\u{0650}"),
    tuple("\u{062f}\u{0650}\u{0651}abc", "\u{062f}\u{0651}\u{0650}"),
    // [é] vs [e + `]
    tuple("\u{00e9}abc", "\u{0065}\u{0301}"),
    tuple("\u{00e9}abc", "e"),
    tuple("abc\u{00e9}", "\u{0065}\u{0301}"),
    tuple("abc\u{00e9}", "e"),
  ];
  foreach($cases as list($haystack, $needle)) {
    var_dump(dict[
      'haystack' => $haystack,
      'needle' => $needle,
      'strip_prefix' => dict[
        'c' => _Str\strip_prefix_l($haystack, $needle, $c),
        'en_US.UTF-8' => _Str\strip_prefix_l($haystack, $needle, $en_us_utf8),
      ],
      'strip_suffix' => dict[
        'c' => _Str\strip_suffix_l($haystack, $needle, $c),
        'en_US.UTF-8' => _Str\strip_suffix_l($haystack, $needle, $en_us_utf8),
      ],
    ]);
  }
}
