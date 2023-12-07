<?hh

interface I<T> {
  public function foo(): T;
}

abstract class D implements I<string> {}

abstract class C extends D implements I<int> {}

class B extends C {
  public function foo(): int {
    return 0;
  }
}

function expects_Istring(I<string> $x): string {
  return $x->foo();
}

<<__EntryPoint>>
function main(): void {
  $b = new B();
  expects_Istring($b);
  // php says:
  // Catchable fatal error: Value returned from function expects_Istring() must be of type string, int given
}
