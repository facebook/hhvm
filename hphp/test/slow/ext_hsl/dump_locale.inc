<?hh

use namespace HH\Lib\{C, Str};
use namespace HH\Lib\_Private\_Locale;

const PORTABLE_CATEGORIES = keyset[
  'LC_COLLATE',
  'LC_CTYPE',
  'LC_MONETARY',
  'LC_NUMERIC',
  'LC_TIME',
  'LC_MESSAGES', // in POSIX standard but not C; not defined on MSVC
];

function dump_locale(string $label, _Locale\Locale $loc): void {
  printf("--- %s ---\n", $label);
  // Only print out the locale categories that exist on every platform
  foreach ($loc->__debugInfo() as $k => $v) {
    if (Str\starts_with($k, 'LC_') && !C\contains_key(PORTABLE_CATEGORIES, $k)) {
      continue;
    }
    printf("  '%s'\t => %s\n", $k, \var_export($v, true));
  }
}
