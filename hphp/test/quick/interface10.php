<?hh

interface I {
  public function a(arraylike $a = null);
}
class C implements I {
  public function a(arraylike $a) {}
}
<<__EntryPoint>> function main(): void {
print "Test begin\n";
print "Test end\n";
}
