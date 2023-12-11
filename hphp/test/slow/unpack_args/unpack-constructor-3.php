<?hh


function handler($kind, $name) :mixed{
  throw new Exception('in handler');
}

class C {
  function __construct($a, $b, $c, $d) {
    echo "in C::__construct\n";
  }
}

function foo($x) :mixed{
  fb_setprofile(handler<>);
  new C(...$x);
}

<<__EntryPoint>>
function main() :mixed{
  try {
    foo(vec[1,2]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
