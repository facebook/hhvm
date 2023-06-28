<?hh

class C<reify T> {
  function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

class D<reify T> extends C<bool> {
  function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
  function h() :mixed{
    $c = new parent();
    $c->f();
  }
}

<<__EntryPoint>> function main(): void {
$d = new D<int>();
$d->h();
}
