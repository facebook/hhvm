<?hh

class X {
  public $exp_info;
  public function __construct(varray $exp_info = null) {
    $this->exp_info = $exp_info ?: varray[];
  }
}

<<__EntryPoint>>
function main_1736() {
$x = new X(varray[0, 1, 2]);
var_dump($x->exp_info);
$x1 = new X(null);
var_dump($x->exp_info);
}
