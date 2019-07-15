<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\getTypeStructure<T>());
  }
  function a() {
    $c = new self();
    $c->f();
  }
}

class D<reify T> extends C<bool> {
  function f() {
    var_dump(HH\ReifiedGenerics\getTypeStructure<T>());
  }
  function h() {
    $this->a();
  }
}

<<__EntryPoint>> function main(): void {
$d = new D<int>();
$d->h();
}
