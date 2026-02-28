<?hh

class C {
  function __construct() {
    print "In C::__construct()\n";
  }
}

class F extends C {}

class I extends F {
  function __construct() {
    print "In I::__construct()\n";
  }
  function test() :mixed{
    C::__construct();
    $X = "C";
    $m = "__construct"; HH\dynamic_meth_caller(C::class, $m)($this); HH\dynamic_meth_caller($X, $m)($this);
    F::__construct();
    I::__construct();
    $X = "F";
    $m = "__construct"; HH\dynamic_meth_caller(F::class, $m)($this); HH\dynamic_meth_caller($X, $m)($this);
    $X = "I";
    $m = "__construct"; HH\dynamic_meth_caller(I::class, $m)($this); HH\dynamic_meth_caller($X, $m)($this);
  }
}

<<__EntryPoint>>
function main_fpush_cls_method() :mixed{
  print "Test begin\n";
  (new I)->test();
  print "Test end\n";
}
