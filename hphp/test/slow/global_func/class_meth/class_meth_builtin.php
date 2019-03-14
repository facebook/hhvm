<?hh

class A { static public function func1() { return 1; } }

function test_legacy_usecase($func) {
  if (is_array($func) && count($func) == 2) {
    var_dump($func[0]);
    var_dump($func[1]);
  }
}

function test_clsmeth_builtins($func) {
  if (HH\is_class_meth($func)) {
    var_dump(HH\class_meth_get_class($func));
    var_dump(HH\class_meth_get_method($func));
  }
}

<<__EntryPoint>>
function main() {
  $c = A::class;
  $f = 'func1';
  print "test on varray:\n";
  test_legacy_usecase(varray[A::class, 'func1']);
  test_clsmeth_builtins(varray[A::class, 'func1']);

  print "test on clsmeth:\n";
  test_legacy_usecase(HH\class_meth(A::class, 'func1'));
  test_clsmeth_builtins(HH\class_meth(A::class, 'func1'));

  // failure
  var_dump(HH\class_meth_get_class(varray[A::class, 'func1']));
}
