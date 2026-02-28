<?hh
class A {
  protected $extension = "html";
  protected $apple = 3;
  protected $foo = null;
  private $orange = null;
}


class B extends A {
  public $foo = null;
  protected $bar = vec[];
}


<<__EntryPoint>>
function main_child_parent_property_ordering() :mixed{
$class = new ReflectionClass("B");
var_dump($class->getProperties());
}
