<?hh

class A {
  static public function f1() {
    return 1;
  }
}

function test_eval($name, $f) {
  var_export($f);
  echo "\n";
  $func = "eval_".$name;
  eval("function ".$func."() { return ".var_export($f, true)."; }");
  $m = $func();
  var_dump($m);
  var_dump($m());
}

<<__EntryPoint>>
function main() {
  test_eval('varray', varray[A::class, 'f1']);
  test_eval('class_meth', HH\class_meth(A::class, 'f1'));
}
