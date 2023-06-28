<?hh

<<A(1),B('foo',varray[42,73])>>
class C {
  <<A(2),B('bar',varray[43,74])>>
  function f() :mixed{
}
}

<<A(3),B('bar',varray[44,75]), C('concat'.'enate')>>
function f() :mixed{
}

<<__EntryPoint>>
function main_2198() :mixed{
$rc = new ReflectionClass('C');
$attrs = $rc->getAttributes();
ksort(inout $attrs);
var_dump($attrs);
$rm = $rc->getMethod('f');
$attrs = $rm->getAttributes();
ksort(inout $attrs);
var_dump($attrs);
$rf = new ReflectionFunction('f');
$attrs = $rf->getAttributes();
ksort(inout $attrs);
var_dump($attrs);
}
