<?hh
class A {
  <<__Memoize>>
  public function &testRefReturn() { return array(1,2,3); }
}


<<__EntryPoint>>
function main_refreturn() {
echo (new A())->testRefReturn();
}
