<?hh

class A {
  public $a = 'foo';
  protected $b = 'bar';
  private $c = 'blah';
  public function aaaa() :mixed{
    var_dump(json_encode($this));
  }
}

<<__EntryPoint>>
function main_1554() :mixed{
$obj = new A();
$obj->aaaa();
}
