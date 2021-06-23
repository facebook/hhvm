<?hh // strict

use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {
  $c = _Locale\get_c_locale();
  $utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);
  $strings = vec['abc', '💩a👀'];

  $offsets = vec[
    tuple(0, -1),
    tuple(0, -2),
    tuple(0, -3),
    tuple(0, 0),
    tuple(0, 1),
    tuple(1, -1),
    tuple(1, 1),
    tuple(1, 2),
    tuple(-1, -1),
    tuple(-1, 0),
    tuple(-1, 1),
    tuple(-2, -1),
    tuple(-2, -2),
    tuple(-2, 1),
    tuple(-3, -1),
    tuple(-3, -2),
    tuple(-3, -3),
    tuple(0, 9999),
  ];
  printf("input\toffset\tlength\tC\tbytes\tUTF-8\tbytes\n");
  foreach($strings as $str) {
    foreach($offsets as list($offset, $length)) {
      $c_slice = _Str\slice_l($str, $offset, $length, $c);
      $utf8_slice =  _Str\slice_l($str, $offset, $length, $utf8);
      printf(
        "%s\t%d\t%d\t%s\t%d\t%s\t%d\n",
        $str,
        $offset,
        $length,
        $c_slice,
        \strlen($c_slice),
        $utf8_slice,
        \strlen($utf8_slice),
      );
    }
  }
}
