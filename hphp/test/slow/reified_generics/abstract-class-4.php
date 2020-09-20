<?hh

abstract class A<reify T> {}

class C extends A<int> {}


<<__EntryPoint>>
function main() {
  $c = new C();
  var_dump($c is A<int>);
  var_dump($c is A<bool>);
}

