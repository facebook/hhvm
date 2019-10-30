<?hh

$b = 123;
if ($b) {
  include '1461-1.inc';
} else {
  include '1461-2.inc';
}

class Exception2 extends Exception1 {}

function foo() {
  $e = new Exception();
  try {
    throw new Exception2();
  }
  catch (Exception $e) {
    var_dump($e->getCode());
  }
}
foo();
