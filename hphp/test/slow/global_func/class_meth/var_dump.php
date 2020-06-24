<?hh

class A {
  static public function f1() {
    return 1;
  }
}

function test_eval($name, $f) {
  var_export($f);
  echo "\n";
  var_dump($f);
  echo "\n";
}

<<__EntryPoint>>
function main() {
  test_eval('varray', varray[A::class, 'f1']);
  test_eval('class_meth', HH\class_meth(A::class, 'f1'));
}
