<?hh

function __autoload($a) {
  var_dump($a);
  if ($a == 'A') {
    include 'autoload7-1.inc';
  }
}

<<__EntryPoint>> function main(): void {
  $a = '\\A';
  new $a;
}
