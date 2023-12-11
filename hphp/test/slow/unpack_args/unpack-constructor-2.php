<?hh

class C {
  function __construct($a, $b) {
    throw new Exception("in C::__construct");
  }
}

function foo($x) :mixed{
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
