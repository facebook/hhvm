<?hh

interface I {
  public function a(AnyArray $a = null):mixed;
}
class C implements I {
  public function a(AnyArray $a) :mixed{}
}
<<__EntryPoint>> function main(): void {
print "Test begin\n";
print "Test end\n";
}
