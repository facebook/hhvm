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
    $this->x = vec[new Obj];
  }

  public function getVal() :mixed{
    return $this->x[0]->val;
  }
}

function main(CGetM $k) :mixed{
  return $k->getVal();
}


<<__EntryPoint>>
function main_cgetm_complex() :mixed{
main(new CGetM());
}
