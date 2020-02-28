<?hh

class C {}

class D {
  private $array;

  function __construct() {
    $this->array = darray[];
  }

  function __get($key) {
    $this->array[$key] = $this->array[$key] ?? null;
    return $this->array[$key];
  }
}

function main() {
  $c = new C();
  $c->foo = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[42]);
  $c->bar += 42;
  $c->baz++;
  var_dump($c);

  $d = new D();
  $d->bar += 42;
  $d->baz++;
  var_dump($d);
}


<<__EntryPoint>>
function main_dyn_props() {
main();
}
