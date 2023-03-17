<?hh

enum class E : int {
  int A = 42;
  int B = 0;
}

function f(HH\EnumClass\Label<E, int> $label = E#A) : int {
  return E::valueOf($label);
}

<<__EntryPoint>>
function main() : void {
  echo f() . "\n";
  echo f(#B) . "\n";
}
