<?hh

class A { static public function func1() :mixed{ return 1; } }

function test_legacy_usecase($func) :mixed{
  if (is_array($func) && count($func) == 2) {
    var_dump($func[0]);
    var_dump($func[1]);
  }
}

function test_clsmeth_builtins($func) :mixed{
  if (HH\is_class_meth($func)) {
    var_dump(HH\class_meth_get_class($func));
    var_dump(HH\class_meth_get_method($func));
  }
}

<<__EntryPoint>>
function main() :mixed{
  $c = A::class;
  $f = 'func1';
  print "test on varray:\n";
  test_legacy_usecase(vec[A::class, 'func1']);
  test_clsmeth_builtins(vec[A::class, 'func1']);

  print "test on clsmeth:\n";
  test_legacy_usecase(A::func1<>);
  test_clsmeth_builtins(A::func1<>);

  // failure
  var_dump(HH\class_meth_get_class(vec[A::class, 'func1']));
}
