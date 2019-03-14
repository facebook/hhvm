<?hh

function __autoload($a) {
  var_dump($a);
  if ($a == 'A') {
    include 'autoload7-1.inc';
  }
}

function main() {
  $a = '\\A';
  new $a;
}

main();
