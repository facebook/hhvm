<?hh

class A {
  function foo(inout $test) :mixed{
    $test[3] = 10;
  }
}

<<__EntryPoint>>
function main_1213() :mixed{
$obj = new A();
$method = 'foo';
$aa = dict[];
$obj->$method(inout $aa);
var_dump($aa);
}
