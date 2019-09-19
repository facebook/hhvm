<?hh

interface I {
  public static function foo();
}
class C implements I {
  public static function foo($x) {
    echo 'Hello ' . $x . "\n";
  }
}
<<__EntryPoint>> function main(): void {
print "Test begin\n";
C::foo("5");
print "Test end\n";
}
