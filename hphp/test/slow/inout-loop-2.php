<?hh

class D {
  function __destruct() { echo "destruct!\n"; }
}

function foo(inout int $x) {
  foreach (array(new D) as $bar) {
    try {
      return $x;
    } finally {
      $x = $x + 1;
    }
  }
}

function main() {
  $y = 0;
  foo(inout $y);
  echo "done\n";
}

main();
