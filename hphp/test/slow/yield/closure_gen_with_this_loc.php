<?hh

class X {
  function gen($fn) :mixed{
    return function() use ($fn) {
      yield $fn($this);
    };
  }
}

function test() :mixed{
  $x = new X;
  $f = $x->gen(function($x) { var_dump(get_class($x)); });
  foreach ($f() as $e) {
    var_dump($e);
  }
}

function fiz($x) :mixed{ return false; }


<<__EntryPoint>>
function main_closure_gen_with_this_loc() :mixed{
test();
}
