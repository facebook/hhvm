<?hh

class A {
  static public function f1() {
    return 1;
  }
}

function test_eval($f) {
  var_export($f);
  echo "\n";
  eval("\$m = ".var_export($f, true).";");
  var_dump($m);
  var_dump($m());
}

<<__EntryPoint>>
function main() {
  test_eval(varray[A::class, 'f1']);
  test_eval(HH\class_meth(A::class, 'f1'));
}
