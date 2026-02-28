<?hh

function f<reify Ta, Tb, <<__Soft>> reify Tc, reify Td>() :mixed{}

function g<T>() :mixed{
  f<int, int, string, int>(); // no warning
  f<int, int, T, int>(); // warning
  f<int, int, T, T>(); // warning and then error
}
<<__EntryPoint>> function main(): void {
g();
}
