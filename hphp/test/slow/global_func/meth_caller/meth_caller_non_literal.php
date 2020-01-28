<?hh

class A { function afunc($x) { return $x; } }

function test_builtins($m) {
  if ($m is __SystemLib\MethCallerHelper) {
    var_dump($m->getClassName(), $m->getMethodName());
  }
  if (HH\is_meth_caller($m)) {
    var_dump(\HH\meth_caller_get_class($m), \HH\meth_caller_get_method($m));
  }
}

<<__EntryPoint>>
function main() {
  test_builtins(HH\meth_caller(A::class, "afunc"));
  $carr = varray[A::class];
  $func_n = "afunc";
  foreach ($carr as $c) {
    $m = HH\meth_caller($c, $func_n);
    test_builtins($m);
    var_dump($m(new $c(), 1));
  }
}
