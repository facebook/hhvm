<?hh


function handler($kind, $name) {
  throw new Exception('in handler');
}

class C {
  function __construct($a, $b, $c, $d) {
    echo "in C::__construct\n";
  }
}

function foo($x) {
  fb_setprofile('handler');
  new C(...$x);
}

<<__EntryPoint>>
function main() {
  try {
    foo(varray[1,2]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
