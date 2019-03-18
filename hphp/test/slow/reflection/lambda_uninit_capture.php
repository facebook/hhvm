<?hh

function test() {
  $y = 1;
  $foo = () ==> $x + $y;
  $x = 2;
  $r = new ReflectionFunction($foo);
  var_dump($r->isClosure()); // true
  $foo(); // x is not defined
}


<<__EntryPoint>>
function main_lambda_uninit_capture() {
test();
}
