<?hh

class stringer {
  public function __toString() { throw new Exception("nope\n"); }
}

function foo() {
  $x = new stringer();
  $b = "foo";
  $b .= $x;
}
<<__EntryPoint>> function main(): void {
try {
  foo();
} catch (Exception $e) {
  echo "Caught!\n";
}

print "all ok\n";
}
