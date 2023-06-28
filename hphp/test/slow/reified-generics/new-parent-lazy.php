<?hh

class C<reify T> {
  function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

class D extends C<bool> {
  function h() :mixed{
    $c = new parent();
    $c->f();
  }
}

<<__EntryPoint>> function main(): void {
  $cls = D::class;
  $d = new $cls();
  $d->h();
}
