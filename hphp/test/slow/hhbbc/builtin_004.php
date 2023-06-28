<?hh

class X {}
class Y {}
class Z extends Y {}
function foo(X $x) :mixed{ return get_class($x); }
function foo2(Y $y) :mixed{ return is_string(get_class($y)); }
function foo3($x) :mixed{ if (is_object($x)) return get_class($x); return ""; }

<<__EntryPoint>>
function main_builtin_004() :mixed{
var_dump(foo(new X));
var_dump(foo2(new Z));
var_dump(is_string(foo3(new stdClass)));
}
