<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\getType<T>());
  }
}

class D<reify T> extends C<bool> {
  function f() {
    var_dump(HH\ReifiedGenerics\getType<T>());
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
