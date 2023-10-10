<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {
  $c = _Locale\get_c_locale();
  $en_us = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US", $c);
  $en_us_utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);

  $cases = vec[
    // We're currently treating 'C' as 'special, not-intended for humans', so
    // using it for byte string comparisons works. This means that "C" should
    // sort "a\0a" differently to `a\0b", but 'real' libc locales like en_US
    // should see the string as just `a`.
    //
    // As we're assuming that input is well-formed, if it's actually a C string,
    // the data shouldn't include a null byte, so this no effect on valid
    // "C" locale strings
    tuple("a\0a with null second byte", "a\0b with null second byte"),
    tuple('aa', 'ab'),
    tuple('aa', 'aaa'),
    tuple('aaa', 'aa'),
    tuple('abc', 'Abc'),
    tuple('aaa', 'aäb'),
    tuple('aaa', 'aäa'),
    tuple('aab', 'aäa'),
  ];

  foreach ($cases as list($a, $b)) {
    var_dump(
      dict[
        'a' => $a,
        'b' => $b,
        '"C" strcoll' => _Str\strcoll_l($a, $b, $c),
        '"C" strcasecmp' => _Str\strcasecmp_l($a, $b, $c),
        '"en_US" strcoll' => _Str\strcoll_l($a, $b, $en_us),
        '"en_US" strcasecmp' => _Str\strcasecmp_l($a, $b, $en_us),
        '"en_US.UTF-8" strcoll' => _Str\strcoll_l($a, $b, $en_us_utf8),
        '"en_US.UTF-8" strcasecmp' => _Str\strcasecmp_l($a, $b, $en_us_utf8),
      ]
    );
  }
}
