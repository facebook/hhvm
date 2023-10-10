<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {

  $cases = vec[
    tuple('abc', 'a', 0),
    tuple('abc', 'A', 0),
    tuple('abc', 'b', 0),
    tuple('abc', 'c', 0),
    tuple('abc', 'd', 0),
    tuple('aib', 'i', 0),
    tuple('aib', 'I', 0),
    tuple('aib', 'ı', 0),
    tuple('aib', 'İ', 0),
    tuple('a💩b', 'b', 0),
    tuple('a💩b', '💩', 0),
    tuple('abc', 'abcd', 0),
    tuple('', 'abcd', 0),
    tuple('abcd', '', 0),
    tuple('', '', 0),
    tuple('abab', 'a', 0),
    tuple('abab', 'b', 0),
    tuple('abab', 'A', 0),
    tuple('abab', 'B', 0),
    tuple('abab', 'a', 1),
    tuple('abab', 'a', 3),
    tuple('abab', 'b', -1),
    tuple('abab', 'b', -2),
    tuple('abab', 'ab', -1),
    tuple('abab', 'ab', -2),
    tuple('💩a💩', '💩', -1),
    tuple('💩a💩', '💩', -2),
    tuple('💩a💩', '💩', -3),
  ];

  $c = _Locale\get_c_locale();
  $locales = dict[
    'C' => $c,
    'en UTF8' => _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c),
    'tr UTF8' => _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "tr_TR.UTF-8", $c),
  ];

  $headers = "HaystackNeedle\tOffset\tLocale\tstrpos\tstrrpos\tstripos\tstrripos\n";
  print($headers);
  foreach ($cases as list($haystack, $needle, $offset)) {
    print(str_repeat('-', 80)."\n");
    foreach ($locales as $locale_name => $locale) {
      printf(
        "%s\t%s\t%d\t%s\t%s\t%s\t%s\t%s\n",
        var_export($haystack, true),
        var_export($needle, true),
        $offset,
        $locale_name,
        _Str\strpos_l($haystack, $needle, $offset, $locale),
        _Str\strrpos_l($haystack, $needle, $offset, $locale),
        _Str\stripos_l($haystack, $needle, $offset, $locale),
        _Str\strripos_l($haystack, $needle, $offset, $locale),
      );
    }
  }
  print(str_repeat('-', 80)."\n");
  print($headers);
}
