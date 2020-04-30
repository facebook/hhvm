<?hh

class C {}

<<__EntryPoint>>
function main() {
  $c = new C();
  $c->foo = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[42]);
  $c->bar += 42;
  $c->baz++;
  var_dump($c);
}
