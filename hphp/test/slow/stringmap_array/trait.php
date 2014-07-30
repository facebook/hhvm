<?hh

trait MSArrayTrait {
  public $msarray = msarray(
    'foo' => true,
    'bar' => false,
  );
}

class MyClass {
  use MSArrayTrait;
}

function main() {
  $clazz = new MyClass;
  $clazz->msarray[-34] = 'warning';
  var_dump($clazz->msarray);
}

main();
