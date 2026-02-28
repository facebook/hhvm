<?hh

trait T {
  private $a = 1;
  private static $sa = 1;
  public $pa = 1;
  public static $spa = 1;

  public function t() :mixed{
    var_dump($this->a);
    var_dump(get_object_vars($this));
  }
}

class A {
  use T;

  private $b = 4;
  public $c = 'hi';
  private static $sb = 4;
  public static $sc = 'hi';
}



<<__EntryPoint>>
function main_prop_from_traits() :mixed{
  $a = new A();
  $a->t();

  $cls = new ReflectionClass($a);
  foreach ($cls->getProperties() as $prop) {
    $prop->getAttributes();
  }
}
