<?hh
class A {
  <<__Memoize>>
  public function testArgs(int &$a) { return $a; }
}


<<__EntryPoint>>
function main_refargs() {
echo (new A())->testArgs(1);
}
