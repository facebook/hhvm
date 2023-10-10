<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {
  $inputs = vec[
    tuple('abc', dict['a' => 'r1']),
    tuple('Abc', dict['a' => 'r1']),
    tuple('abba', dict['a' => 'r1']),
    tuple('abc', dict['a' => 'r1', 'b' => 'r2']),
    tuple('abc', dict['ab' => 'r1', 'b' => 'r2']),
    tuple('abc', dict['ab' => 'r1', 'r1' => 'r2']),
    tuple('abba', dict['a' => 'r1', 'r1' => 'r2']),
    tuple('abba', dict['a' => 'R1', 'r1' => 'r2']),
    tuple('aia', dict['I' => 'r']),
    tuple('aia', dict['İ' => 'r']),
    tuple('aia', dict['I' => 'r1', 'r1' => 'r2']),
    tuple('aia', dict['İ' => 'r1', 'r1' => 'r2']),
    // `non_recursive` longest-wins:
    tuple('ababa', dict['a' => '1', 'ab' => '2']),
    tuple('ababa', dict['ab' => '1', 'a' => '2']),
    // `non-recursive` with multiple matches at differing offset
    tuple('HI HELLO world', dict['hi' => 'hello', 'hello' => 'hi']),
  ];

  $c = _Locale\get_c_locale();
  $en_us_utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);
  $tr_tr_utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "tr_TR.UTF-8", $c);
  foreach($inputs as list($haystack, $replacements)) {
    var_dump(dict[
      'haystack' => $haystack,
      'replacements' => $replacements,
      'C' => dict[
        'replace_every_l()' => _Str\replace_every_l($haystack, $replacements, $c),
        'replace_every_nonrecursive_l()' => _Str\replace_every_nonrecursive_l($haystack, $replacements, $c),
        'replace_every_ci_l()' => _Str\replace_every_ci_l($haystack, $replacements, $c),
        'replace_every_nonrecursive_ci_l()' => _Str\replace_every_nonrecursive_ci_l($haystack, $replacements, $c),
      ],
      'en_US.UTF-8' => dict[
        'replace_every_l()' => _Str\replace_every_l($haystack, $replacements, $en_us_utf8),
        'replace_every_nonrecursive_l()' => _Str\replace_every_nonrecursive_l($haystack, $replacements, $en_us_utf8),
        'replace_every_ci_l()' => _Str\replace_every_ci_l($haystack, $replacements, $en_us_utf8),
        'replace_every_nonrecursive_ci_l()' => _Str\replace_every_nonrecursive_ci_l($haystack, $replacements, $en_us_utf8),
      ],
      'tr_TR.UTF-8' => dict[
        'replace_every_l()' => _Str\replace_every_l($haystack, $replacements, $tr_tr_utf8),
        'replace_every_nonrecursive_l()' => _Str\replace_every_nonrecursive_l($haystack, $replacements, $tr_tr_utf8),
        'replace_every_ci_l()' => _Str\replace_every_ci_l($haystack, $replacements, $tr_tr_utf8),
        'replace_every_nonrecursive_ci_l()' => _Str\replace_every_nonrecursive_ci_l($haystack, $replacements, $tr_tr_utf8),
      ],
    ]);
  }

  print("--- Invalid inputs\n");
  $inputs = dict[
    'abc' => 123,
    123 => 'abc',
    '' => 'abc',
  ];
  $encodings = dict['C' => $c, 'UTF8' => $en_us_utf8];
  $funcs = vec[
    _Str\replace_every_l<>,
    _Str\replace_every_ci_l<>,
    _Str\replace_every_nonrecursive_l<>,
    _Str\replace_every_nonrecursive_ci_l<>,
  ];
  foreach ($inputs as $needle => $replacement) {
    foreach($encodings as $name => $encoding) {
      printf("--- %s: %s => %s\n", $name, $needle, $replacement);
      foreach ($funcs as $fun) {
        printf("- %s\n", \HH\fun_get_function($fun));
        try {
          $fun('abc', dict[$needle => $replacement], $encoding);
          printf("Expected exception not thrown!\n");
        } catch (InvalidArgumentException $ex) {
          printf("%s\n", $ex->getMessage());
        }
      }
    }
  }

  print("--- Similar pairs, e.g. due to normalization or capitalization\n");
  $inputs = vec[
    tuple('abc', 'ABC'),
    // [é] vs [e + `]
    tuple("\u{00e9}", "\u{0065}\u{0301}"),
    // [É], [e + `]
    tuple("\u{00c9}", "\u{0065}\u{0301}"),
    // [e], [E + `]
    tuple("\u{00e9}", "\u{0045}\u{0301}"),
    // [É], [é]
    tuple("\u{00c9}", "\u{00e9}"),
  ];
  foreach ($inputs as list($a, $b)) {
    foreach($encodings as $name => $encoding) {
      printf("--- %s: (%s, %s)\n", $name, $a, $b);
      foreach ($funcs as $fun) {
        printf("- %s\n", \HH\fun_get_function($fun));
        try {
          $fun('abc', dict[$a => 'RA', $b => 'RB'], $encoding);
          print("No exception.\n");
        } catch (InvalidArgumentException $ex) {
          printf("%s\n", $ex->getMessage());
        }
      }
    }
  }
}
