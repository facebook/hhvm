<?hh

class A {
  public $a;
  function foo() :mixed{
    $this->bar();
    if ($this is B) {
      $this->b = 1;
    }
    $this->a = 1;
  }
}
class B extends A {
  public $b;
  function bar() :mixed{
}
}
function main() :mixed{
  $b = new B;
  $b->foo();
  var_dump($b);
}

<<__EntryPoint>>
function main_1327() :mixed{
main();
}
