<?hh

class Obj {
  public $val;
  public function __construct() {
    $this->val = "string";
  }
}

class CGetM {
  private $x;

  public function __construct() {
    $this->x = varray[new Obj];
  }

  public function getVal() {
    return $this->x[0]->val;
  }
}

function main(CGetM $k) {
  return $k->getVal();
}


<<__EntryPoint>>
function main_cgetm_complex() {
main(new CGetM());
}
