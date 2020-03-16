<?hh

namespace {
class A { function f() {} function B() {} }
class B { function f() {} function B() {} }
class C { function f() {} function B() {} }
class D { function f() {} function B() {} }
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
class B { function f() {} }
class C { function f() {} }
class D { function f() {} }
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

final abstract class MethCallerStrWrap {
    public static function get(mixed $fun_meth_or_string): ?string {
        if (\HH\is_meth_caller($fun_meth_or_string)) {
          return
            \HH\meth_caller_get_class($fun_meth_or_string) . "::" .
            \HH\meth_caller_get_method($fun_meth_or_string);
        }
        return null;
    }
}

function testFunction(mixed $fun_meth_or_string) {
  $function_name = MethCallerStrWrap::get($fun_meth_or_string);
  return $function_name;
}

function get_name(string $name, string $n): string {
  return $name . $n;
}

class A {
  private $map = darray[];
  function set($k, $v) {
    $this->map[$k] = $v;
  }
  function f() {}
}
$o = new A();
$func_name = testFunction(\HH\meth_caller(get_name('A', 'a'), 'B'));
$o->set("a", $func_name);
$func_name = testFunction(\HH\meth_caller(get_name('A', 'b'), 'B'));
$o->set("b", $func_name);
$func_name = testFunction(\HH\meth_caller('C', 'B'));
$o->set("c", $func_name);
$func_name = testFunction(\HH\meth_caller('D', 'B'));
$o->set("d", $func_name);

// bad argument
\HH\meth_caller_get_class(\HH\fun('afunc'));
}
