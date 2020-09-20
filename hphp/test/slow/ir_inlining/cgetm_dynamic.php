<?hh

class CGetMDynProp {
  public function __construct() {
    $this->str = "yolo";
  }
  public function getString() {
 return $this->str;
 }
}

function test6() {
  $obj = new CGetMDynProp();
  echo $obj->getString();
  echo "\n";
}


<<__EntryPoint>>
function main_cgetm_dynamic() {
test6();
}
