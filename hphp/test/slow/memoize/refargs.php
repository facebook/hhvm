<?hh
class A {
  <<__Memoize>>
  public function testArgs(inout int $a) :mixed{ return $a; }
}
