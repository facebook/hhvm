<?hh

class C<reify T> {}

function f<reify T>(C<T> $a) {
  return $a;
}

f<int>(new C<int>);
f<int>(new C<string>);
