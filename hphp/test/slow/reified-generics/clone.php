<?hh

class C<reify T> {
  function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}
<<__EntryPoint>> function main(): void {
$c = new C<int>();
$c->f();

$d = clone($c);
$d->f();
}
