<?hh

class X {
  public $pub_var = null;
  public $pub_set = varray[];
  private $priv_var = 2;
  function __get($name) {
    echo 'get: ';
 var_dump($name);
 return $name == 'buz' ? 1 : varray[];
  }
  function __isset($name) {
    echo 'isset: ';
 var_dump($name);
    return $name == 'baz' || $name == 'buz';
  }
}

<<__EntryPoint>>
function main_690() {
$x = new X;
var_dump(isset($x->foo));
var_dump(isset($x->baz));
var_dump(isset($x->buz));
var_dump(isset($x->pub_var));
var_dump(isset($x->pub_set));
var_dump(isset($x->priv_var));
unset($x->pub_var);
var_dump(isset($x->pub_var));
}
