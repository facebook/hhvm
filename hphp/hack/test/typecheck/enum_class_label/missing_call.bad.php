<?hh

enum class E : int {
  int A = 42;
}

class C {
  public function f(HH\EnumClass\Label<E, int> $label) : void {
    echo "f " . E::valueOf($label) . "\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $c = new C();
  $c->f#A; // should be $c->f(#A) or $c->f#A() with experimental syntax
}
