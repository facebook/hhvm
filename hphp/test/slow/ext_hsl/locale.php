<?hh

use namespace HH\Lib\_Private\_Locale as L;
use namespace HH\Lib\{C, Str, Vec};

<<__EntryPoint>>
function test_hsl_locale(): void {
  require_once(__DIR__.'/dump_locale.inc');
  $c = L\get_c_locale();
  dump_locale('C', $c);

  $en_gb = L\newlocale_category(L\LC_ALL, 'en_GB.UTF-8', $c);
  dump_locale('en_GB.UTF-8', $en_gb);

  $c_utf8 = L\newlocale_category(L\LC_CTYPE, 'C', $en_gb);
  dump_locale('en_GB with UTF-8 ctype', $c_utf8);

  $fr_partial = L\newlocale_mask(
    L\LC_MONETARY_MASK | L\LC_NUMERIC_MASK,
    'fr_FR.UTF-8',
    $en_gb
  );
  dump_locale('fr partial', $fr_partial);

  L\set_request_locale($fr_partial);
  dump_locale('request locale', L\get_request_locale());

  print("--- Request locale from setlocale() ---\n");
  // semi-colon separated foo=bar;herp=derp
  $pairs = setlocale(LC_ALL, "0")
    |> Str\split($$, ';')
    |> Vec\sort($$);
  foreach ($pairs as $pair) {
    list($category, $locale) = Str\split($pair, '=');
    if (!C\contains_key(PORTABLE_CATEGORIES, $category)) {
      continue;
    }
    printf("   %s\t=> %s\n", $category, $locale);
  }
}
