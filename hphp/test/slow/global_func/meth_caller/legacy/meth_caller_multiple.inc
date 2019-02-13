<?hh

namespace Ans {

class A { function afunc($x) { return $x * 2; } }

$x = \HH\meth_caller(A::class, "afunc");
var_dump($x, $x(new A(), 1));

$x = \HH\meth_caller(\Ans\A::class, "afunc");
var_dump($x, $x(new A(), 2));

}
