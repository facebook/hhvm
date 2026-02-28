<?hh

interface I {
  public function foo(int $x, optional bool $y, optional string $z):void;
}
class C implements I {
  public function foo(int $x, bool $y = false, string $z = 'a'):void {
    var_dump($x);
    var_dump($y);
    var_dump($z);
  }
}
function test_optional(I $i):void {
  $i->foo(3);
  $i->foo(4, true);
  $i->foo(5, true, 'b');
}

<<__EntryPoint>>
function main():void {
  test_optional(new C());
}
