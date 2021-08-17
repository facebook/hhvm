<?hh // strict

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
        'replace_every_ci_l()' => _Str\replace_every_ci_l($haystack, $replacements, $c),
      ],
      'en_US.UTF-8' => dict[
        'replace_every_l()' => _Str\replace_every_l($haystack, $replacements, $en_us_utf8),
        'replace_every_ci_l()' => _Str\replace_every_ci_l($haystack, $replacements, $en_us_utf8),
      ],
      'tr_TR.UTF-8' => dict[
        'replace_every_l()' => _Str\replace_every_l($haystack, $replacements, $tr_tr_utf8),
        'replace_every_ci_l()' => _Str\replace_every_ci_l($haystack, $replacements, $tr_tr_utf8),
      ],
    ]);
  }
}
