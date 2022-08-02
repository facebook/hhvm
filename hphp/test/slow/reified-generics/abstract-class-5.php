<?hh

abstract class A {}

class C<reify T> extends A {
  function f() {
    var_dump($this->{'86reified_prop'});
  }
}

<<__EntryPoint>>
function main() {
    $c = new C<int>();
    $c->f();
}
