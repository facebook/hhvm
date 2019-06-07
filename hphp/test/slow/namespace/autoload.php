<?hh

namespace {
  function __autoload($a) {
    \var_dump($a);
    if ($a == 'A') {
      include 'autoload.inc';
    }
  }
}

namespace A {
  function main() {
    $a = '\\A';
    new $a;
  }

  main();
}
