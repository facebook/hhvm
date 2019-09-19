<?hh

interface I {
  public function a(array $a = null);
}
class C implements I {
  public function a(array $a) {}
}
<<__EntryPoint>> function main(): void {
print "Test begin\n";
print "Test end\n";
}
