<?hh

class C<reify T> {}

function f<reify T>(mixed $x): T {
  return $x;
}

f<reify C<int>>(new C<reify int>());
f<reify C<int>>(new C<reify string>());
