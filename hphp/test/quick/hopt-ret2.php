<?hh

class C {
  public $y=5;
  function foo($x) :mixed{
    return $x + $this->y;
  }
}
<<__EntryPoint>> function main(): void {
$o = new C;
echo $o->foo(3) . "\n";
}
