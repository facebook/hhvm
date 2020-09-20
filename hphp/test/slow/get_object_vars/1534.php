<?hh

class Base{
  public    $aaa = 1;
  protected $bbb = 2;
  private   $ccc = 3;
}
class Child extends Base{
  public    $ddd = 4;
}
class Unrelated{
  function foo($obj) {
    var_dump(get_object_vars($obj));
  }
}

<<__EntryPoint>>
function main_1534() {
$base_obj = new Base();
$child_obj = new Child();
$unrelated_obj = new Unrelated();
$unrelated_obj->foo($child_obj);
$unrelated_obj->foo($base_obj);
}
