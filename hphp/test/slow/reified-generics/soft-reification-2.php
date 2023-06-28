<?hh

class C<reify Ta, Tb, <<__Soft>> reify Tc, reify Td> {}

function g<T>() :mixed{
  new C<int, int, string, int>(); // no warning
  new C<int, int, T, int>(); // warning
  new C<int, int, T, T>(); // warning and then error
}
<<__EntryPoint>> function main(): void {
g();
}
