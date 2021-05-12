<?hh // strict

use namespace HH\Lib\_Private\_Locale as L;
use namespace HH\Lib\{C, Str, Vec};

const PORTABLE_CATEGORIES = keyset[
  'LC_COLLATE',
  'LC_CTYPE',
  'LC_MONETARY',
  'LC_NUMERIC',
  'LC_TIME',
  'LC_MESSAGES', // in POSIX standard but not C; not defined on MSVC
];

function dump(string $label, L\Locale $loc): void {
  printf("--- %s ---\n", $label);
  // Only print out the locale categories that exist on every platform
  foreach ($loc->__debugInfo() as $k => $v) {
    if (Str\starts_with($k, 'LC_') && !C\contains_key(PORTABLE_CATEGORIES, $k)) {
      continue;
    }
    printf("  '%s'\t => %s\n", $k, \var_export($v, true));
  }
}

<<__EntryPoint>>
function test_hsl_locale(): void {
  $c = L\get_c_locale();
  dump('C', $c);

  $en_gb = L\newlocale_category(L\LC_ALL, 'en_GB.UTF-8', $c);
  dump('en_GB.UTF-8', $en_gb);

  $c_utf8 = L\newlocale_category(L\LC_CTYPE, 'C', $en_gb);
  dump('en_GB with UTF-8 ctype', $c_utf8);

  $fr_partial = L\newlocale_mask(
    L\LC_MONETARY_MASK | L\LC_NUMERIC_MASK,
    'fr_FR.UTF-8',
    $en_gb
  );
  dump('fr partial', $fr_partial);

  L\set_request_locale($fr_partial);
  dump('request locale', L\get_request_locale());

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
