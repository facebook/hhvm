<?hh

class A {
  private function __call($x, $y) {
    echo "fail\n";
  }

  public function foo() {
    $this->bar();
  }
}

class B extends A {
  public function bar() {
    echo "ok\n";
  }
}

<<__EntryPoint>>
function main() {
  $b = new B();
  $b->foo();
}
