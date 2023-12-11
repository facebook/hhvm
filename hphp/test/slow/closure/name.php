<?hh

// Inside a function
function testOne() :mixed{
  return () ==> debug_backtrace();
}

function testTwo() :mixed{
  () ==> 2;
  return () ==> debug_backtrace();
}

function testNested() :mixed{
  $c1 = () ==> {
    $c2 = () ==> debug_backtrace();
    return $c2();
  };
  return $c1;
}

function testTwoNested() :mixed{
  () ==> 2;
  $c1 = () ==> {
    () ==> 1;
    $c2 = () ==> debug_backtrace();
    return $c2();
  };
  return $c1;
}

class Test {
  public static function testStatic() :mixed{
    return () ==> debug_backtrace();
  }
  public function testMethod() :mixed{
    return () ==> debug_backtrace();
  }
}
<<__EntryPoint>> function main(): void {
$closures = vec[
  testOne(),
  testTwo(),
  testNested(),
  testTwoNested(),
  Test::testStatic(),
  (new Test())->testMethod(),
];

foreach ($closures as $t) {
  var_dump(array_map($x ==> $x['function'],
                     $t()));
}
}
