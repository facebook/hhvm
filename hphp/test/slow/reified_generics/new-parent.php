<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

class D<reify T> extends C<bool> {
  function f() {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
  function h() {
    $c = new parent();
    $c->f();
  }
}

<<__EntryPoint>> function main(): void {
$d = new D<int>();
$d->h();
}
