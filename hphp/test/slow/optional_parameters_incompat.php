<?hh

interface I {
  public function foo(int $x, optional bool $y, optional string $z):void;
}
class C implements I {
  public function foo(int $x, bool $y = false, string $z = 'a'):void {
    var_dump($z);
  }
}
class E implements I {
  // Rejected by Hack, should also be rejected by HHVM
  public function foo(int $x, bool $y, string $z):void {
    var_dump($z);
  }
}
<<__EntryPoint>>
function main():void {
  $x = new C();
  $y = new E();
}
