<?hh

class X {
  function gen($fn) {
    return function() use ($fn) {
      yield $fn($this);
    };
  }
}

function test() {
  $x = new X;
  $f = $x->gen(function($x) { var_dump(get_class($x)); });
  foreach ($f() as $e) {
    var_dump($e);
  }
}

function fiz($x) { return false; }


<<__EntryPoint>>
function main_closure_gen_with_this_loc() {
test();
}
