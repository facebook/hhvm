<?hh

abstract class A {}

class C<reify T> extends A {
  function f() :mixed{
    var_dump($this->{'86reified_prop'});
  }
}

<<__EntryPoint>>
function main() :mixed{
    $c = new C<int>();
    $c->f();
}
