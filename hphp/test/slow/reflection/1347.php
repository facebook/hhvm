<?hh

class A {
}
class B extends A {
}
class C extends B {
}

<<__EntryPoint>>
function main_1347() {
  $a = new A;
  $b = new B;
  var_dump(is_a('A', 'A', true));
  var_dump(is_a('A', 'A', false));
  var_dump(is_a('B', 'A', true));
  var_dump(is_a('A', 'B', true));
  var_dump(is_a('C', 'A', true));
}
