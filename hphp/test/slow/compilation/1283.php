<?hh

class A {
 function f($a) :mixed{
}
 }

<<__EntryPoint>>
function main_1283() :mixed{
$obj = new A;
$obj->f(date('m/d/y H:i:s', 123456789));
$v = HH\Lib\Legacy_FIXME\cast_for_arithmetic(date("m",123456789))+1;
}
