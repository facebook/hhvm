<?hh

class C<reify T> {}

function f<reify T>(T $a) {
  return $a;
}

f<reify C<int>>(new C<reify int>);
f<reify C<int>>(new C<reify string>);
