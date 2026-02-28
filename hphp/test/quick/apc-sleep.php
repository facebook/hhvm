<?hh
class Foo {
  public $foo;
  public $bar;
  public function __construct($f, $b) {
    echo "Constructing a Foo\n";
    $this->foo = $f;
    $this->bar = $b;
  }
  public function __sleep() :mixed{
    echo "I'm going to sleep\n";
    return vec['foo'];
  }
  public function __wakeup() :mixed{
    echo "I'm waking up\n";
  }
}

<<__EntryPoint>> function main(): void {
  $foo1 = new Foo(1, 2);
  var_dump($foo1);
  apc_store('x', $foo1);
  unset($foo1);
  $foo2 = __hhvm_intrinsics\apc_fetch_no_check('x');
  var_dump($foo2);
}
