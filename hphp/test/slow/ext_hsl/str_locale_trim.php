<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};

function do_test(_Locale\Locale $loc, vec<(string, ?string)> $inputs) :mixed{
  foreach ($inputs as list($str, $what)) {
    var_dump(dict[
      'input' => $str,
      'to trim' => $what,
      'trim_l()' => _Str\trim_l($str, $what, $loc),
      'trim_left_l()' => _Str\trim_left_l($str, $what, $loc),
      'trim_right_l()' => _Str\trim_right_l($str, $what, $loc),
    ]);
  }
}

<<__EntryPoint>>
function main(): void {
  print("----- C -----\n");
  $c = _Locale\get_c_locale();
  $c_inputs = vec[
    tuple('   foo   ', null),
    tuple('   foo   ', 'ab'),
    tuple('', ''),
    tuple('   ', ''),
    tuple('   ', 'a'),
    tuple('afoob', 'ab'),
    tuple('💩foo💩', '💩'),
    tuple('💩foo💩', '😀'), // these two emoji have some common bytes
  ];
  do_test($c, $c_inputs);
  print("----- UTF-8 -----\n");
  $utf_8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);
  do_test($utf_8, $c_inputs);
  do_test($utf_8, vec[
    tuple("\u{000a} foo \u{000a}", null),
    tuple("\u{000a} foo \u{000a}", 'ab'),
    tuple("\u{000a} foo \u{000a}", "\u{000a} "),
    tuple("\u{000a} foo \u{000a}\t", "\u{000a} "),
  ]);
}
