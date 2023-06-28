<?hh

class stringer {
  public function __toString() :mixed{ throw new Exception("nope\n"); }
}

function foo() :mixed{
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
