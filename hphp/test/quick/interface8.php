<?hh

interface A {
  public function a(AnyArray $a = null):mixed;
}
class B implements A {
  public function a($a) :mixed{}
}

<<__EntryPoint>> function main(): void {
print "Test end\n";
}
