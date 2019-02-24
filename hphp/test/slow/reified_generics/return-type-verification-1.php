<?hh

class C<reify T> {}

function f(mixed $x): C<int> {
  return $x;
}

f(new C<int>());
f(new C<string>());
