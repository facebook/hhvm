<?hh

namespace Ans\Bns;
include_once("meth_caller_multiple.inc");

class A { function afunc($x) { return $x * 3; } }

$x = \HH\meth_caller(\Ans\A::class, "afunc");
var_dump($x, $x(new \Ans\A(), 1));

// A::class is in Ans\Bns namespace
$x = \HH\meth_caller(A::class, "afunc");
var_dump($x, $x(new A(), 1));
