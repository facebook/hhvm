<?hh

class C<reify T1, T2, reify T3> {}

function g<T>() {
  new C<reify int, string, T>();
}

g();
