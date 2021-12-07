<?hh

class A {
  final public function __construct(private $h)[] {}
  public function foo() { return $this->h['r'] as E; }
}

<<__EntryPoint>>
function main() {
  HH\autoload_set_paths(
    dict['class' => dict['e' => 'resolved-autoload.inc']],
    __DIR__.'/',
  );

  $a = new A(shape('r' => 'key2'));
  var_dump($a->foo());
}
