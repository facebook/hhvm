<?hh

abstract class A<reify T> {
  function f() :mixed{
    var_dump($this->{'86reified_prop'});
  }
}

class C<T> extends A<int> {}

<<__EntryPoint>>
function main() :mixed{
  $c = new C<string>();
  $c->f();
}
