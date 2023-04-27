<?hh

interface I {
  abstract const int X;
}

class C implements I {
  const int X = 42;
}

abstract enum class A : mixed {
  abstract const type T as I;
}

enum class B : mixed extends A {
  const type T = C;
}

function test<T as A>(classname<T> $ec): int {
  $ts = type_structure($ec, 'T');
  $cn = $ts['classname'];
  return $cn::X;
}

<<__EntryPoint>>
function main(): void {
  echo test(B::class);
  echo "\n";
}
