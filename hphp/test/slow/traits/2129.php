<?hh

function show_without_extra_vardump_nonsense($arr) :mixed{
  echo 'array (' . count($arr) . ") {\n";
  foreach ($arr as $val) {
    echo "  $val\n";
  }
  echo "}\n";
}
function do_wonderful_things_with($r) :mixed{
  echo "yall know what time it is. time to show you some properties\n";
  $props = vec[];
  foreach ($r->getProperties() as $prop) {
    $props[] = $prop->getName();
  }
  asort(inout $props);
  show_without_extra_vardump_nonsense($props);
  $meths = vec[];
  echo "yall know what time it is now too. time to show you some methods\n";
  foreach ($r->getMethods() as $meth) {
    $meths[] = $meth->getName();
  }
  asort(inout $meths);
  show_without_extra_vardump_nonsense($meths);
}

trait T {
  private $priv;
  protected $prot;
  public $pub;

  private static $priv_st;
  protected static $prot_st;
  public static $pub_st;

  private function fpriv() :mixed{
}
  protected function fprot() :mixed{
}
  public function fpub() :mixed{
}

  private static function fpriv_st() :mixed{
}
  protected static function fprot_st() :mixed{
}
  public static function fpub_st() :mixed{
}
}

trait U {
  public $foo;
  public static $static;

  public function ffoo() :mixed{
}
  public static function fstatic() :mixed{
}
}

class C {
  use T;

  private $c_priv;
}

class D extends C {
  use U;

  public $class_prop;
  public function class_method() :mixed{
}

}


<<__EntryPoint>>
function main_2129() :mixed{
$r = new ReflectionClass('C');
do_wonderful_things_with($r);

$r = new ReflectionClass('D');
do_wonderful_things_with($r);
}
