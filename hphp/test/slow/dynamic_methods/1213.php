<?hh

class A {
  function foo(inout $test) {
    $test[3] = 10;
  }
}

<<__EntryPoint>>
function main_1213() {
$obj = new A();
$method = 'foo';
$aa = varray[];
$obj->$method(inout $aa);
var_dump($aa);
}
