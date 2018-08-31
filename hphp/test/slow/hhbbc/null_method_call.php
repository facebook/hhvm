<?hh

class Foo {
  public function bar() {}
}

class Baz {
  public static bool $x = false;
  private $foo;

  public function run() {
    $foo = $this->getFoo();
    $foo->bar();
    echo "unreachable\n";
    return $foo;
  }

  private function getFoo() {
    if (self::$x) return null;
    if (!$this->foo) {
      $this->foo = new Foo();
    }

    return $this->foo;
  }

}


<<__EntryPoint>>
function main_null_method_call() {
Baz::$x = true;
try {
  (new Baz())->run();
} catch (Exception $e) {
  echo $e->getMessage()."\n";
}
}
