<?hh

class C<reify Ta, Tb, <<__Soft>> reify Tc, reify Td> {}

function g<T>() {
  new C<reify int, int, reify string, reify int>(); // no warning
  new C<reify int, int, T, reify int>(); // warning
  new C<reify int, int, T, T>(); // warning and then error
}

g();
