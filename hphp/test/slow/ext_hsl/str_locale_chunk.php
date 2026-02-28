<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};
use namespace HH\Lib\{Str, Vec};

<<__EntryPoint>>
function main(): void {
  $c = _Locale\get_c_locale();
  $en_us_utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);
  $in = "😎😎😎😎";
  var_dump(dict[
    'in' => $in,
    'c' => _Str\chunk_l($in, 1, $c),
    'en_us.UTF-8' => _Str\chunk_l($in, 1, $en_us_utf8),
  ]);
  var_dump(dict[
    'in' => $in,
    'c' => _Str\chunk_l($in, 2, $c),
    'en_us.UTF-8' => _Str\chunk_l($in, 2, $en_us_utf8),
  ]);
  $in = "😎😎😎😎😎";
  var_dump(dict[
    'in' => $in,
    'c' => _Str\chunk_l($in, 2, $c),
    'en_us.UTF-8' => _Str\chunk_l($in, 2, $en_us_utf8),
  ]);

  $in = 'the quick brown fox jumped over the lazy dog';

  var_dump(_Str\chunk_l($in, 1, $c));
  var_dump(_Str\chunk_l($in, 1, $c) === _Str\chunk_l($in, 1, $en_us_utf8));
  var_dump(_Str\chunk_l($in, 2, $c));
  var_dump(_Str\chunk_l($in, 2, $c) === _Str\chunk_l($in, 2, $en_us_utf8));

  $invalids = vec[
    tuple('libc', 0, $c),
    tuple('icu', 0, $en_us_utf8),
    tuple('libc', -1, $c),
    tuple('icu', -1, $en_us_utf8),
  ];
  foreach($invalids as list($name, $size, $locale)) {
    printf("%s with a chunk size of %d:\n", $name, $size);
    try {
      var_dump(_Str\chunk_l($in, $size, $locale));
    } catch (Throwable $e) {
      printf("  Exception %s: %s\n", \get_class($e), $e->getMessage());
    }
  }
}
