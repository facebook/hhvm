<?hh

interface I {
  public function foo(int $x, optional bool $y, optional string $z):void;
}

class C implements I {
  public function foo(int $x, bool $y = false, string $z = 'a'):void {
    var_dump($z);
  }
}
function testit(I $i):void {
  $i->foo(3);
  $i->foo(4,true);
  $i->foo(4,true,"B");
}
<<__EntryPoint>>
function main():void {
  testit(new C());
}
