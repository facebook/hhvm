<?hh

class Foo {
  public function bar() :mixed{}
}

class Baz {
  public static bool $x = false;
  private $foo;

  public function run() :mixed{
    $foo = $this->getFoo();
    $foo->bar();
    echo "unreachable\n";
    return $foo;
  }

  private function getFoo() :mixed{
    if (self::$x) return null;
    if (!$this->foo) {
      $this->foo = new Foo();
    }

    return $this->foo;
  }

}


<<__EntryPoint>>
function main_null_method_call() :mixed{
Baz::$x = true;
try {
  (new Baz())->run();
} catch (Exception $e) {
  echo $e->getMessage()."\n";
}
}
