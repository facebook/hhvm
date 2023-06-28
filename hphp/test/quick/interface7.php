<?hh

interface I {
  public function foo($x, $y):mixed;
}
class C implements I {
  public function foo($x) :mixed{
    echo 'Hello ' . $x . "\n";
  }
}
<<__EntryPoint>> function main(): void {
print "Test begin\n";
$o = new C;
$o->foo("5");
print "Test end\n";
}
