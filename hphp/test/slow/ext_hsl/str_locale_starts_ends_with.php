<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {
  $c = _Locale\get_c_locale();
  $en_us_utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);
  $tr_tr_utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "tr_TR.UTF-8", $c);

  $cases = vec[
    tuple('abc', 'a'),
    tuple('abc', 'A'),
    tuple('abc', 'c'),
    tuple('abc', 'C'),
    tuple('abc', 'ab'),
    tuple('abc', 'bc'),
    tuple('αλφα', 'Αλφα'),
    tuple('iab', 'i'),
    tuple('iab', 'I'),
    tuple('iab', 'ı'),
    tuple('iab', 'İ'),
    tuple('', ''),
    tuple('', 'a'),
    tuple('a', ''),
    tuple('a', 'needle longer than haystack'),
    // Differing byte order but still equivalent
    tuple("\u{062f}\u{0650}\u{0651}", "\u{062f}\u{0651}\u{0650}"),
  ];

  foreach($cases as list($haystack, $needle)) {
    var_dump(dict[
      'haystack' => $haystack,
      'needle' => $needle,
      'starts_with' => dict[
        'c' => _Str\starts_with_l($haystack, $needle, $c),
        'en_US.UTF-8' => _Str\starts_with_l($haystack, $needle, $en_us_utf8),
        'tr_TR.UTF-8' => _Str\starts_with_l($haystack, $needle, $tr_tr_utf8),
      ],
      'starts_with_ci' => dict[
        'c' => _Str\starts_with_ci_l($haystack, $needle, $c),
        'en_US.UTF-8' => _Str\starts_with_ci_l($haystack, $needle, $en_us_utf8),
        'tr_TR.UTF-8' => _Str\starts_with_ci_l($haystack, $needle, $tr_tr_utf8),
      ],
      'starts_with' => dict[
        'c' => _Str\starts_with_l($haystack, $needle, $c),
        'en_US.UTF-8' => _Str\starts_with_l($haystack, $needle, $en_us_utf8),
        'tr_TR.UTF-8' => _Str\starts_with_l($haystack, $needle, $tr_tr_utf8),
      ],
      'ends_with' => dict[
        'c' => _Str\ends_with_l($haystack, $needle, $c),
        'en_US.UTF-8' => _Str\ends_with_l($haystack, $needle, $en_us_utf8),
        'tr_TR.UTF-8' => _Str\ends_with_l($haystack, $needle, $tr_tr_utf8),
      ],
      'ends_with_ci' => dict[
        'c' => _Str\ends_with_ci_l($haystack, $needle, $c),
        'en_US.UTF-8' => _Str\ends_with_ci_l($haystack, $needle, $en_us_utf8),
        'tr_TR.UTF-8' => _Str\ends_with_ci_l($haystack, $needle, $tr_tr_utf8),
      ],
    ]);
  }
}
