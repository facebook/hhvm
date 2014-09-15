<?hh
class A {
  <<__Memoize>>
  public function testArgs(int &$a) { return $a; }
}

echo (new A())->testArgs(1);
