<?hh

class A { function afunc($x) { return $x; } }
class B { function bfunc($x) { return $x * 2; } }
class C extends B { function cfunc($x, $y) { return $x + $y; } }

// test in empty scope
$x = HH\meth_caller(A::class, "afunc");
var_dump($x(new A(), 1));

// test in function scope
function test_duplicate_meth_caller() {
  var_dump(HH\meth_caller("B", "bfunc")(new B(), 2));
  var_dump(HH\meth_caller(B::class, "bfunc")(new B(), 3));
}
test_duplicate_meth_caller();

// test class inheritance
var_dump(HH\meth_caller(B::class, "bfunc")(new C(), 4));
var_dump(HH\meth_caller(B::class, "cfunc")(new C(), 4, 5));

// test in class method scope
class D {
  function dfunc($x) {
    var_dump(HH\meth_caller(B::class, "bfunc")(new B(), $x));
  }
}
new D()->dfunc(5);

// failure case
var_dump(HH\meth_caller(D::class, "dfunc")(new B(), 6));
