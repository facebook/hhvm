<?hh
class A {
  <<__Memoize>>
  public function testArgs(?string $a) { return $a; }
}

echo (new A())->testArgs('foo');
