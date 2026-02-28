<?hh

class C {
}
class D {
  public function __construct($f) {
    $this->map = $f;
  }
}
class E {
  protected $map;
  public function __construct($f) {
    $this->map = $f;
  }
  public function getMap() :mixed{
    return $this->map;
  }
}

<<__EntryPoint>>
function main_1822() :mixed{
$f = new stdClass();
$arr = vec[new E($f), new D($f)];
apc_store('ggg', $arr);
$arr2 = __hhvm_intrinsics\apc_fetch_no_check('ggg');
var_dump($arr[0]->getMap());
var_dump($arr[1]->map);
var_dump($arr2[0]->getMap());
var_dump($arr2[1]->map);
}
