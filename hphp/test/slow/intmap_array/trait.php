<?hh

trait MIArrayTrait {
  public $miarray = miarray(
    1 => 2,
    3 => 4,
  );
}

class MyClass {
  use MIArrayTrait;
}

function main() {
  $clazz = new MyClass;
  $clazz->miarray['warning'] = true;
  var_dump($clazz->miarray);
}

main();
