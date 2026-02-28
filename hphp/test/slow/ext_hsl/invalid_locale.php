<?hh
use namespace HH\Lib\_Private\_Locale;
use namespace HH\Lib\Locale;

<<__EntryPoint>>
function main(): void {
    require_once(__DIR__.'/dump_locale.inc');
  $c = _Locale\get_c_locale();
  $en_us = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, 'en_US.UTF-8', $c);
  \dump_locale('en_US.UTF-8', $en_us);
  try {
    $utf16 = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, 'en_US.UTF-16', $c);
  } catch (Locale\InvalidLocaleException $e) {
    printf("Exception: %s\n", $e->getMessage());
  }

  try {
    $invalid_country = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, 'xx_XX.UTF-8', $c);
  } catch (Locale\InvalidLocaleException $e) {
    printf("Exception: %s\n", $e->getMessage());
  }
}
