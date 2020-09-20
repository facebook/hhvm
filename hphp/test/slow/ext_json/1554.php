<?hh

class A {
  public $a = 'foo';
  protected $b = 'bar';
  private $c = 'blah';
  public function aaaa() {
    var_dump(json_encode($this));
  }
}

<<__EntryPoint>>
function main_1554() {
$obj = new A();
$obj->aaaa();
}
