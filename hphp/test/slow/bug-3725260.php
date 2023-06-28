<?hh

// Test that concat binops with un-used results are not incorrectly optimized
// away if they can invoke side-effects. See task #3725260.

class A {
  public function __toString()[] :mixed{ echo "__toString()\n"; return "heh"; }
}


<<__EntryPoint>>
function main_bug_3725260() :mixed{
$a = new A();
$b = new A();
$a . $b;

echo "done\n";
}
