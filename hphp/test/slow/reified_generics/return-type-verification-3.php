<?hh

class C<reify T> {}

function f<reify T>(mixed $x): T {
  return $x;
}

f<C<int>>(new C<int>());
f<C<int>>(new C<string>());
