<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
  function a() {
    $c = new static();
    $c->f();
  }
}

class D extends C<bool> {
  function h() {
    $this->a();
  }
}

<<__EntryPoint>> function main(): void {
  $cls = D::class;
  $d = new $cls();
  $d->h();
}
