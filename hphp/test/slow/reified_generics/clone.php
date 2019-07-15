<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\getTypeStructure<T>());
  }
}
<<__EntryPoint>> function main(): void {
$c = new C<int>();
$c->f();

$d = clone($c);
$d->f();
}
