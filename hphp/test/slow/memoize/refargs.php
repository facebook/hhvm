<?hh
class A {
  <<__Memoize>>
  public function testArgs(inout int $a) { return $a; }
}
