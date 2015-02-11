<?hh

// Inside a function
function testOne() {
  return () ==> debug_backtrace();
}

function testTwo() {
  () ==> 2;
  return () ==> debug_backtrace();
}

function testNested() {
  $c1 = () ==> {
    $c2 = () ==> debug_backtrace();
    return $c2();
  };
  return $c1;
}

function testTwoNested() {
  () ==> 2;
  $c1 = () ==> {
    () ==> 1;
    $c2 = () ==> debug_backtrace();
    return $c2();
  };
  return $c1;
}

class Test {
  public static function testStatic() {
    return () ==> debug_backtrace();
  }
  public function testMethod() {
    return () ==> debug_backtrace();
  }
}

$closures = array(
  testOne(),
  testTwo(),
  testNested(),
  testTwoNested(),
  Test::testStatic(),
  (new Test())->testMethod(),
);

foreach ($closures as $t) {
  var_dump(array_map($x ==> $x['function'],
                     $t()));
}
