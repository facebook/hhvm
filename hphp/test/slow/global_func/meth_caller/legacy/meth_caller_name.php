<?hh

namespace {
$x = \HH\meth_caller(A::class, "f");
\var_dump(
  $x, \HH\is_meth_caller($x),
  \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
$f = () ==> {
  $x = \HH\meth_caller(B::class, "f");
  \var_dump(
    $x, \HH\is_meth_caller($x),
    \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
};
$f();

function afunc() {
  $x = \HH\meth_caller(C::class, "f");
  \var_dump(
    $x, \HH\is_meth_caller($x),
    \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
}
afunc();

class Acls {
  function bfunc() {
    $x = \HH\meth_caller(D::class, "f");
    \var_dump(
      $x, \HH\is_meth_caller($x),
      \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
  }
}
new Acls()->bfunc();
}

namespace Ans {
$x = \HH\meth_caller(A::class, "f");
\var_dump(
  $x, \HH\is_meth_caller($x),
  \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
$f = () ==> {
  $x = \HH\meth_caller(B::class, "f");
  \var_dump(
    $x, \HH\is_meth_caller($x),
    \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
};
$f();

function afunc() {
  $x = \HH\meth_caller(C::class, "f");
  \var_dump(
    $x, \HH\is_meth_caller($x),
    \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
}
afunc();

class Acls {
  function bfunc() {
    $x = \HH\meth_caller(D::class, "f");
    \var_dump(
      $x, \HH\is_meth_caller($x),
      \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
  }
}
new Acls()->bfunc();
\var_dump(\HH\is_meth_caller(new Acls()));

$x = \HH\meth_caller(Acls::class, "bfunc");
\var_dump($x->getClassName(), $x->getMethodName());
}
