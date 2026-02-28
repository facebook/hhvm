<?hh

class A {
  final public function __construct(private $h)[] {}
  public function foo() :mixed{ return $this->h['r'] as E; }
}

<<__EntryPoint>>
function main() :mixed{
  $a = new A(shape('r' => 'key2'));
  var_dump($a->foo());
}
