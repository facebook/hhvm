<?hh

use namespace HH\Lib\_Private\_Locale as L;
use namespace HH\Lib\_Private\_Str;

<<__EntryPoint>>
function main(): void {
  $en_gb = L\newlocale_all('en_GB.UTF-8');
  $fr_fr = L\newlocale_all('fr_FR.UTF-8');
  print _Str\vsprintf_l($en_gb, "%f\n", vec[1.23]);
  print _Str\vsprintf_l($fr_fr, "%f\n", vec[1.23]);
  print "Trying empty locale (env)\n";
  try {
    L\newlocale_all('');
  } catch (InvalidArgumentException $e) {
    \var_dump(\get_class($e), $e->getMessage());
  }
  print "Trying '0' locale (return)\n";
  try {
    L\newlocale_all('0');
  } catch (InvalidArgumentException $e) {
    \var_dump(\get_class($e), $e->getMessage());
  }
  print "Trying int 0 locale (type error)\n";
  try {
    L\newlocale_all(0);
  } catch (Throwable $e) {
    printf("%s\n", $e->getMessage());
  }
}
