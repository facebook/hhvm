<?hh

class A {
  static public function f1() :mixed{
    return 1;
  }
}

function test_eval($name, $f) :mixed{
  var_export($f);
  echo "\n";
  var_dump($f);
  echo "\n";
}

<<__EntryPoint>>
function main() :mixed{
  test_eval('varray', vec[A::class, 'f1']);
  test_eval('class_meth', A::f1<>);
}
