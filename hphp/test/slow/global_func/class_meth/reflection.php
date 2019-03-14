<?hh

class A {
  static public function f1() {
    return 1;
  }
}

function func($p0 = HH\class_meth(A::class, 'f1')) {
}

<<__EntryPoint>>
function main() {
  $x = new ReflectionFunction('func');
  var_dump($x->getParameters());
}
