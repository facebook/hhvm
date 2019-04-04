<?hh

namespace Ans {

class A { function afunc($x) { return $x * 2; } }

$x = \HH\meth_caller(A::class, "afunc");
\var_dump($x, $x(new A(), 1));
\var_dump(\HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));

$x = \HH\meth_caller(\Ans\A::class, "afunc");
\var_dump($x, $x(new A(), 2));
\var_dump(\HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
}
