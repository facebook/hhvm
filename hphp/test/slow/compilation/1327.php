<?hh

class A {
  public $a;
  function foo() {
    $this->bar();
    if ($this is B) {
      $this->b = 1;
    }
    $this->a = 1;
  }
}
class B extends A {
  public $b;
  function bar() {
}
}
function main() {
  $b = new B;
  $b->foo();
  var_dump($b);
}

<<__EntryPoint>>
function main_1327() {
main();
}
