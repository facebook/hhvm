<?hh

abstract class A<reify T> {
  function f($x) :mixed{
    var_dump($this->{'86reified_prop'});
  }
}

class C extends A<int> {}

<<__EntryPoint>>
function main() :mixed{
  $c = new C();
  $c->f(1);
}
