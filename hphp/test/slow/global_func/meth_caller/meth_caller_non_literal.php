<?hh

class A { function afunc($x) :mixed{ return $x; } }

function test_builtins($m) :mixed{
  if ($m is __SystemLib\MethCallerHelper) {
    var_dump($m->getClassName(), $m->getMethodName());
  }
  if (HH\is_meth_caller($m)) {
    var_dump(\HH\meth_caller_get_class($m), \HH\meth_caller_get_method($m));
  }
}

<<__EntryPoint>>
function main() :mixed{
  test_builtins(HH\meth_caller(A::class, "afunc"));
  $carr = vec[A::class];
  $func_n = "afunc";
  foreach ($carr as $c) {
    $m = new __SystemLib\MethCallerHelper($c, $func_n);
    test_builtins($m);
    var_dump($m(new $c(), 1));
  }
}
