<?hh

class Foo {
  private $private;
  protected $protected;
  public $public;
  public function __unset($prop) { echo "__unset($prop)\n"; }
}


<<__EntryPoint>>
function main_public_prop_unset() {
$obj = new Foo();

unset($obj->private);
unset($obj->private);
unset($obj->protected);
unset($obj->protected);
unset($obj->public);
unset($obj->public);
}
