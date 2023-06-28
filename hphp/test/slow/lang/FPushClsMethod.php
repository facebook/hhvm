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
    $m = "__construct"; C::$m(); $X::$m();
    F::__construct();
    I::__construct();
    $X = "F";
    $m = "__construct"; F::$m(); $X::$m();
    $X = "I";
    $m = "__construct"; I::$m(); $X::$m();
  }
}

<<__EntryPoint>>
function main_fpush_cls_method() :mixed{
  print "Test begin\n";
  (new I)->test();
  print "Test end\n";
}
