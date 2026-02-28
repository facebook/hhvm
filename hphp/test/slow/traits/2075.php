<?hh

trait T {
  public function foo() :mixed{
    $this->bar(function() {
yield 1;
 yield 2;
 yield 3;
}
);
  }
  public function bar(Closure $c) :mixed{
    $a = $c();
    foreach ($a as $b) {
      echo $b."\n";
    }
  }
}
class A {
 use T;
 }

<<__EntryPoint>>
function main_2075() :mixed{
$a = new A();
$a->foo();
}
