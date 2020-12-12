<?hh

function f<reify Ta, Tb, <<__Soft>> reify Tc, reify Td>() {}

function g<T>() {
  f<int, int, string, int>(); // no warning
  f<int, int, T, int>(); // warning
  f<int, int, T, T>(); // warning and then error
}
<<__EntryPoint>> function main(): void {
g();
}
