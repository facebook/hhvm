<?hh

abstract class c {
  public static $arr = varray[];
  function g() {
    $cl = new ReflectionClass(get_class($this));
    $p = $cl->getProperty('arr');
    return $p->getValue();
  }
}
abstract class aa extends c {
  public function get_arr() {
    $actions = parent::get_arr();
    return $actions;
  }
}
class a extends aa {
  public static $arr = varray['v'];
}

<<__EntryPoint>>
function main_1355() {
$x = new a;
var_dump($x->g());
}
