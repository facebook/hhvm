<?hh

interface IRx {}
class Foo {}
class Bar {
  <<__Rx, __OnlyRxIfImpl(IRx::class)>>
  public function test(): void {}
}

<<__Rx, __AtMostRxAsArgs>>
function my_test_function(<<__OnlyRxIfImpl(IRx::class)>> Foo $foo): void {
  $foo as Bar;
  $foo->test();
}
