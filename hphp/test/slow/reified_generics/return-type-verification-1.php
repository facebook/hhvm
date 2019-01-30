<?hh

class C<reify T> {}

function f(mixed $x): C<int> {
  return $x;
}

f(new C<reify int>());
f(new C<reify string>());
