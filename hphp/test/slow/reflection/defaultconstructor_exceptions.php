<?hh

class D {
  // default constructor
}

class P {
  public function __construct() {
    echo __METHOD__, ' ', get_class($this), "\n";
  }
}
class C extends P {
  // inherited constructor
}

function reflect($class_name) :mixed{
  $c = new $class_name('a', 1, 2);

  $rc = new ReflectionClass($class_name);

  try {
    $c2 = $rc->newInstance('a', 1, 2);
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
  try {
    $c2 = $rc->newInstanceArgs(vec['a', 1, 2]);
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
}

function main() :mixed{
  reflect('D');
  reflect('C');
  reflect('P');
}

<<__EntryPoint>>
function main_defaultconstructor_exceptions() :mixed{
main();
}
