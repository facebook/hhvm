<?hh

class A {
  private function __construct() {
  }
  public static function make() :mixed{
    $r = new ReflectionClass('A');
    var_dump($r->isInstantiable());
    var_dump($r->newInstanceWithoutConstructor());
    try {
      $r->newInstance();
    } catch (Exception $e) {
      var_dump(get_class($e));
      var_dump($e->getMessage());
    }
  }
}


<<__EntryPoint>>
function main_private_new() :mixed{
A::make();
$r = new ReflectionClass('A');
var_dump($r->isInstantiable());
var_dump($r->newInstanceWithoutConstructor());

try {
  $r->newInstance();
} catch (Exception $e) {
  var_dump(get_class($e));
  var_dump($e->getMessage());
}
}
