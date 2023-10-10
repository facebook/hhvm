<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {
  $c = _Locale\get_c_locale();
  $en_us_utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);
  $cases = vec[
    tuple('abc', 4, '-'),
    tuple('abc', 6, 'ab'),
    tuple('abc', 4, '123'),
    tuple('abc', 4, '💩'),
    tuple('💩abc', 4, '-'),
    tuple('💩a', 3, '-'),
    tuple('💩a', 4, '-'),
    tuple('💩', 3, '💩'),
    tuple("😀😀😀😀", 5, '!'),
    tuple("😀😀😀😀", 6, '!'),
  ];
  foreach($cases as list($str, $len, $pad)) {
    var_dump(dict[
      'str' => $str,
      'len' => $len,
      'pad' => $pad,
      'pad_left_l()' => dict[
        'C' => _Str\pad_left_l($str, $len, $pad, $c),
        'en_US.UTF-8' => _Str\pad_left_l($str, $len, $pad, $en_us_utf8),
      ],
      'pad_right_l()' => dict[
        'C' => _Str\pad_right_l($str, $len, $pad, $c),
        'en_US.UTF-8' => _Str\pad_right_l($str, $len, $pad, $en_us_utf8),
      ],
    ]);
  }
}
