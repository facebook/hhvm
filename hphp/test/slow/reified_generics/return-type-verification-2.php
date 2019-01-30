<?hh

class C<reify T> {}

function f<reify T>(mixed $x): C<T> {
  return $x;
}

f<reify int>(new C<reify int>());
f<reify int>(new C<reify string>());
