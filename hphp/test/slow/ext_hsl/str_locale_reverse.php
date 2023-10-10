<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {
  $c = _Locale\get_c_locale();
  $utf8 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, "en_US.UTF-8", $c);
  $abc = 'abc';
  $emoji = '💩a👀';

  printf("%s\t%s\t%s\n", $abc, _Str\reverse_l($abc, $c), _Str\reverse_l($abc, $utf8));
  printf("%s\t%s\t%s\n", $emoji, _Str\reverse_l($emoji, $c), _Str\reverse_l($emoji, $utf8));
}
