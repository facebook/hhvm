<?hh

namespace {
class A { function f() :mixed{} function B() :mixed{} }
class B { function f() :mixed{} function B() :mixed{} }
class C { function f() :mixed{} function B() :mixed{} }
class D { function f() :mixed{} function B() :mixed{} }

function afunc() :mixed{
  $x = \HH\meth_caller(C::class, "f");
  \var_dump(
    $x, \HH\is_meth_caller($x),
    \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
}

class Acls {
  function bfunc() :mixed{
    $x = \HH\meth_caller(D::class, "f");
    \var_dump(
      $x, \HH\is_meth_caller($x),
      \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
  }
}

function test() :mixed{
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

  afunc();

  new Acls()->bfunc();
}

<<__EntryPoint>>
function main() :mixed{
  \test();
  \Ans\test();
}
}

namespace Ans {
class B { function f() :mixed{} }
class C { function f() :mixed{} }
class D { function f() :mixed{} }

function afunc() :mixed{
  $x = \HH\meth_caller(C::class, "f");
  \var_dump(
    $x, \HH\is_meth_caller($x),
    \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
}

class Acls {
  function bfunc() :mixed{
    $x = \HH\meth_caller(D::class, "f");
    \var_dump(
      $x, \HH\is_meth_caller($x),
      \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
  }
}

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

function testFunction(mixed $fun_meth_or_string) :mixed{
  $function_name = MethCallerStrWrap::get($fun_meth_or_string);
  return $function_name;
}





class A {
  private $map = dict[];
  function set($k, $v) :mixed{
    $this->map[$k] = $v;
  }
  function f() :mixed{}
}

function test() :mixed{
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

  afunc();

  new Acls()->bfunc();
  \var_dump(\HH\is_meth_caller(new Acls()));

  $o = new A();




  $func_name = testFunction(\HH\meth_caller(C::class, 'B'));
  $o->set("c", $func_name);
  $func_name = testFunction(\HH\meth_caller(D::class, 'B'));
  $o->set("d", $func_name);

  // bad argument
  \HH\meth_caller_get_class(\afunc<>);
}
}
