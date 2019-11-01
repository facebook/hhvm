<?hh

class C {
  function __construct($a, $b) {
    throw new Exception("in C::__construct");
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
