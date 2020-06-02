<?hh

namespace Ans\Bns;

<<__EntryPoint>> function main(): void {
  include_once("meth_caller_multiple.inc");
  \Ans\foo();

  include_once("meth_caller_multiple.a.inc");

$x = \HH\meth_caller(\Ans\A::class, "afunc");
\var_dump($x, $x(new \Ans\A(), 1));
\var_dump(
  \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));

// A::class is in Ans\Bns namespace
$x = \HH\meth_caller(A::class, "afunc");
\var_dump($x, $x(new A(), 1));
\var_dump(
  \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
}
