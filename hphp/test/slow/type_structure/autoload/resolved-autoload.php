<?hh

class A {
  final public function __construct(private $h)[] {}
  public function foo() { return $this->h['r'] as E; }
}

<<__EntryPoint>>
function main() {
  $a = new A(shape('r' => 'key2'));
  var_dump($a->foo());
}
