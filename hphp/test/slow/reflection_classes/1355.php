<?hh

abstract class c {
  public static $arr = vec[];
  function g() :mixed{
    $cl = new ReflectionClass(get_class($this));
    $p = $cl->getProperty('arr');
    return $p->getValue();
  }
}
abstract class aa extends c {
  public function get_arr() :mixed{
    $actions = parent::get_arr();
    return $actions;
  }
}
class a extends aa {
  public static $arr = vec['v'];
}

<<__EntryPoint>>
function main_1355() :mixed{
$x = new a;
var_dump($x->g());
}
