<?hh

function test() :mixed{
  try {
    $classes = get_declared_classes();
    $t = 0;
    foreach ($classes as $class) {
      $r = new ReflectionClass($class);
      $t += HH\Lib\Legacy_FIXME\cast_for_arithmetic(count($r->getMethods()));
    }
    var_dump('ok');
  }
 catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1344() :mixed{
test();
}
