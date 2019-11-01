<?hh

class C {
  function __construct($a, $b, $c, $d) {
    echo "in C::__construct\n";
  }
}

function foo($x) {
  new C(...$x);
}

<<__EntryPoint>>
function main() {
  try {
    foo(array(1,2));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
