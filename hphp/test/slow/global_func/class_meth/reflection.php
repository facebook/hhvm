<?hh

class A {
  static public function f1() {
    return 1;
  }
}

function func($p0 = A::f1<>) {
}

<<__EntryPoint>>
function main() {
  $x = new ReflectionFunction('func');
  var_dump($x->getParameters());
}
