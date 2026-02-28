<?hh

class A { function afunc($x) :mixed{ return $x; } }
class B { function bfunc($x) :mixed{ return $x * 2; } function cfunc($x, $x) :mixed{} }
class C extends B { function cfunc($x, $y) :mixed{ return $x + $y; } }

// test in function scope
function test_duplicate_meth_caller() :mixed{
  var_dump(HH\meth_caller(B::class, "bfunc")(new B(), 3));
}

// test in class method scope
class D {
  function dfunc($x) :mixed{
    var_dump(HH\meth_caller(B::class, "bfunc")(new B(), $x));
  }
}
<<__EntryPoint>>
function entrypoint_meth_caller(): void {

  // test in empty scope
  $x = HH\meth_caller(A::class, "afunc");
  var_dump($x(new A(), 1));
  test_duplicate_meth_caller();

  // test class inheritance
  var_dump(HH\meth_caller(B::class, "bfunc")(new C(), 4));
  var_dump(HH\meth_caller(B::class, "cfunc")(new C(), 4, 5));
  new D()->dfunc(5);

  // failure case
  var_dump(HH\meth_caller(D::class, "dfunc")(new B(), 6));
}
