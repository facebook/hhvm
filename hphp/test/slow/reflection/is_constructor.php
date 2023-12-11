<?hh

interface TestInterface {
  public function __construct();
}

abstract class TestClassImplementingInterface implements TestInterface {
}

class TestConcreteClass {
  public function __construct() { }
}

abstract class TestAbstractClass{
  abstract public function __construct();
}

trait TestTrait {
  abstract public function __construct();
}

function main() :mixed{
  $classes = vec[
    'TestClassImplementingInterface', // false
    'TestInterface', // false
    'TestConcreteClass', // true
    'TestAbstractClass', // true
    'TestTrait' // true
  ];

  $out = dict[];
  foreach ($classes as $class) {
    $rc = (new ReflectionClass($class));
    $out[$class] = $rc->getMethod('__construct')->isConstructor();
  }

  ksort(inout $out);
  var_dump($out);
}


<<__EntryPoint>>
function main_is_constructor() :mixed{
main();
}
