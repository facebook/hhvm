<?hh
<<__EntryPoint>>
function main(): void {
  setlocale(LC_ALL, 'fr_FR');
  var_dump(sprintf('%.2f', 1.23));
  var_dump(HH\Lib\_Private\_Str\vsprintf_l(null, '%.2f', vec[1.23]));
  var_dump(HH\Lib\_Private\_Str\vsprintf_l(HH\Lib\Locale\create('fr_FR'), '%.2f', vec[1.23]));
}
