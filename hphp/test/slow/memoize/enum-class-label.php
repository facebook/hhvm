<?hh

enum class E: int {
  int A = 1;
  int B = 2;
}

<<__Memoize>>
function foo(HH\EnumClass\Label<E, int> $label) : int {
  return E::valueOf($label);
}

<<__EntryPoint>>
function main(): void {
  echo foo(E#A) . "\n";
  echo foo(E#B) . "\n";
  echo foo(E#B) . "\n";
  echo foo(E#A) . "\n";
}
