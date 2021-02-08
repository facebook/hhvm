<?hh

class C {}

<<__EntryPoint>>
function main() {
  $c = new C();
  $c->foo = darray(vec[42]);
  $c->bar += 42;
  $c->baz++;
  var_dump($c);
}
