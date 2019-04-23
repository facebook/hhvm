<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\getType<T>());
  }
}

$c = new C<int>();
$c->f();

$d = clone($c);
$d->f();
