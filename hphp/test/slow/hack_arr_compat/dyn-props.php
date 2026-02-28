<?hh

class C {}

function error_boundary(inout $x, $fn) :mixed{
  try {
    $fn(inout $x);
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
}

<<__EntryPoint>>
function main() :mixed{
  $c = new C();
  $c->foo = darray(vec[42]);
  $c->bar ??= 0;
  $c->bar += 42;
  error_boundary(inout $c, (inout $o) ==> $o->baz++);
  var_dump($c);
}
