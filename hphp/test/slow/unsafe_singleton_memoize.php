<?hh
class Foo implements UNSAFESingletonMemoizeParam {
  public function getInstanceKey(): string {
    return "foo";
  }
}

<<__Memoize>>
function test(Foo $x) : Foo {
  echo "in test\n";
  return $x;
}

<<__EntryPoint>>
function main() : void {
 $x = new Foo();
 $y = new Foo();
 test($x);
 test($y);
 echo "Done\n";
}
