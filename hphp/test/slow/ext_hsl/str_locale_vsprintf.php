<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {
  $c = _Locale\get_c_locale();

  $locales = dict['missing' => null, 'C' => $c];
  foreach (vec['en_US', 'en_US.UTF-8', 'fr_FR', 'fr_FR.UTF-8'] as $name) {
    $locales[$name] = _Locale\newlocale_mask(_Locale\LC_NUMERIC_MASK, $name, $c);
  }
  _Locale\set_request_locale($locales['fr_FR']);
  $locales['request'] = _Locale\get_request_locale();


  foreach ($locales as $name => $locale) {
    printf("%12s\t%s\n", $name, _Str\vsprintf_l($locale, '%f', vec[123]));
  }
}
