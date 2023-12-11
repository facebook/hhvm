<?hh

class A {
  private $b = 'b';
  protected $c = 'c';
  public $d = 'd';
}
class E {
  static private $f = 'f';
  static protected $g = 'g';
  static public $h = 'h';
}

function getProps($class, $obj) :mixed{
  $ret = dict[];
  foreach ((new ReflectionClass($class))->getProperties() as $key => $prop) {
    $values = vec[];

    $key = $prop->getName();

    $prop->setAccessible(true);
    $values[] = $prop->getValue($obj);

    $prop->setValue($obj, 'newval');
    $values[] = $prop->getValue($obj);

    $ret[$key] = $values;
  }
  return $ret;
}


<<__EntryPoint>>
function main_set_accessible() :mixed{
$ret = array_merge(getProps('A', new A), getProps('E', 'E'));
ksort(inout $ret);
var_dump($ret);
}
