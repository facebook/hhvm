<?hh

abstract class A<reify T> {
  function f($x) {
    var_dump($this->{'86reified_prop'});
  }
}

class C extends A<int> {}

<<__EntryPoint>>
function main() {
  $c = new C();
  $c->f(1);
}
