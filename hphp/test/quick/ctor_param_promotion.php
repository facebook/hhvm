<?hh

//
// this works
//
class A {
  public $c;
  public function __construct(protected $a, public $b, $arg) {
    $this->c = $arg;
  }

  public function getA() :mixed{
    return $this->a;
  }
}
<<__EntryPoint>> function main(): void {
$a = new A('hi', 3, dict[]);
foreach ($a as $k => $v) {
  var_dump($k, $v);
}
var_dump($a->getA());
}
