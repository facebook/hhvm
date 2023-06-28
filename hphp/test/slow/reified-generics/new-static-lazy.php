<?hh

class C<reify T> {
  function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
  function a() :mixed{
    $c = new static();
    $c->f();
  }
}

class D extends C<bool> {
  function h() :mixed{
    $this->a();
  }
}

<<__EntryPoint>> function main(): void {
  $cls = D::class;
  $d = new $cls();
  $d->h();
}
