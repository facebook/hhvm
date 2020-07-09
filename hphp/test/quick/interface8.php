<?hh

interface A {
  public function a(arraylike $a = null);
}
class B implements A {
  public function a($a) {}
}

<<__EntryPoint>> function main(): void {
print "Test end\n";
}
