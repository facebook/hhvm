<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {
  $inputs = vec[
    tuple('foobar', 'herp', 'derp'),
    tuple('foobar', 'foo', 'herp'),
    tuple('foobarfoo', 'foo', 'herp'),
    tuple('FoObArFoO', 'foo', 'herp'),
    tuple('foobar', 'Foo', 'derp'),
    tuple('foobar', 'bar', 'herp'),
    tuple('foobar', 'Bar', 'herp'),
    tuple('fOoBaR', 'foo', 'herp'),
    tuple('fOoBaR', 'bar', 'herp'),
    // [é] vs [e + `]
    tuple("\u{00e9}AbC", "\u{0065}\u{0301}", "R"),
    // [É], [e + `]
    tuple("\u{00c9}AbC", "\u{0065}\u{0301}", "R"),
    // [e], [E + `]
    tuple("\u{00e9}AbC", "\u{0045}\u{0301}", "R"),
    // [É], [é]
    tuple("\u{00c9}AbC", "\u{00e9}", "R"),
    // Turkish i
    tuple('fghijkl', 'i', '!'),
    tuple('fghijkl', 'I', '!'),
    tuple('fghijkl', 'İ', '!'),
    tuple('fghijkl', 'ı', '!'),
    tuple('FGHIJKL', 'ı', '!'),
  ];

  $c = _Locale\get_c_locale();
  $en_us_utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);
  $tr_tr_utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "tr_TR.UTF-8", $c);
  foreach($inputs as list($haystack, $needle, $replacement)) {
    var_dump(dict[
      'haystack' => $haystack,
      'needle' => $needle,
      'replacement' => $replacement,
      'C' => dict[
        'replace_l()' => _Str\replace_l($haystack, $needle, $replacement, $c),
        'replace_ci_l()' => _Str\replace_ci_l($haystack, $needle, $replacement, $c),
      ],
      'en_US.UTF-8' => dict[
        'replace_l()' => _Str\replace_l($haystack, $needle, $replacement, $en_us_utf8),
        'replace_ci_l()' => _Str\replace_ci_l($haystack, $needle, $replacement, $en_us_utf8),
      ],
      'tr_TR.UTF-8' => dict[
        'replace_l()' => _Str\replace_l($haystack, $needle, $replacement, $tr_tr_utf8),
        'replace_ci_l()' => _Str\replace_ci_l($haystack, $needle, $replacement, $tr_tr_utf8),
      ],
    ]);
  }
}
