<?hh

class C<reify T> {}

function f<reify T>(C<T> $a) {
  return $a;
}

f<reify int>(new C<reify int>);
f<reify int>(new C<reify string>);
