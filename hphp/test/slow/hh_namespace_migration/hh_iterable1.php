<?hh

// Test that we can define our own custom Iterable interface
// as long as we're not in the top-level namespace.
// Also, a type alias for the classname

namespace Test;

type classname = int;
interface Iterable {
  function foo(classname $x):void;
}

class C implements Iterable {
  public function foo(classname $c):void { echo "C::foo\n"; }
}
function asIterable(Iterable $i):void {
  $i->foo(3);
}
function main() {
  $c = new C();
  asIterable($c);
}


<<__EntryPoint>>
function main_hh_iterable1() {
main();
}
