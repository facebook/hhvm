<?hh

class A {
  public function foo() :mixed{ }
  public $bar0 = 0;
  public $bar1 = 1;
}
<<__EntryPoint>> function main(): void {
$a = new A;
$b = dict[ 0 => 'A', 1 => 'B' ];
list ($a->bar0,  $a->bar1) = $b;
list ($a->foo(), $a->bar1) = $b;
}
