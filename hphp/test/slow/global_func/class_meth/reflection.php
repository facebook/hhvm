<?hh

class A {
  static public function f1() :mixed{
    return 1;
  }
}

function func($p0 = A::f1<>) :mixed{
}

<<__EntryPoint>>
function main() :mixed{
  $x = new ReflectionFunction('func');
  var_dump($x->getParameters());
}
