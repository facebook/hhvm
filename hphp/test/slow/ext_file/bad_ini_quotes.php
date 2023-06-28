<?hh

function main() :mixed{
  $path = __DIR__.'/bad_ini_quotes.ini';
  $x = file_get_contents($path);
  $y = parse_ini_string($x, true);
  var_dump($y);
}


<<__EntryPoint>>
function main_bad_ini_quotes() :mixed{
main();
}
