<?hh

use namespace HH\Lib\_Private\{_Locale, _Str};
use namespace HH\Lib\{Str, Vec};

function dump(string $locale, string $str): void {
  $c = _Locale\get_c_locale();
  $loc = _Locale\newlocale_mask(_Locale\LC_ALL_MASK, $locale, $c);

  $outs = shape(
    'input' => $str,
    'bytes' => _Str\strlen_l($str, $c),
    'chars' => _Str\strlen_l($str, $loc),
    'lower' => _Str\lowercase_l($str, $loc),
    'upper' => _Str\uppercase_l($str, $loc),
    'title' => _Str\titlecase_l($str, $loc),
    'fold' => _Str\foldcase_l($str, $loc),
  );
  printf("----- '%s' - %s ----\n", $str, $locale);
  if (_Str\strlen_l($str, $c) <= 6) {
    print(Str\join(Vec\keys($outs), "\t")."\n");
    print(Str\join($outs, "\t")."\n");
  } else {
    var_dump($outs);
  }
}

<<__EntryPoint>>
function main(): void {
  print("------ no locale ------\n");
  \var_dump(_Str\lowercase_l('iI'));
  \var_dump(_Str\uppercase_l('iI'));
  \var_dump(_Str\titlecase_l('iI'));
  \var_dump(_Str\foldcase_l('iI'));
  \var_dump(_Str\strlen_l('iI'));
  dump('en_US.UTF-8', 'iİıI');
  dump('tr_TR.UTF-8', 'iİıI');
  // ... and of course ...
  dump('en_US.UTF-8', '😀 foo 💩');
  // Words like 'of' used to be in the icu data and kept lowercase for
  // titlecase depending on the locale, but these 'stopwords' were removed in
  // 2012.
  //
  // Title case is still locale-specific due to issues like the Turkish i above
  dump('en_US.UTF-8', 'the foo of bar');
  // TODO: invalid locales
}
