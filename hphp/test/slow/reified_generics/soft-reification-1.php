<?hh

function f<reify Ta, Tb, <<__Soft>> reify Tc, reify Td>() {}

function g<T>() {
  f<reify int, int, reify string, reify int>(); // no warning
  f<reify int, int, T, reify int>(); // warning
  f<reify int, int, T, T>(); // warning and then error
}

g();
