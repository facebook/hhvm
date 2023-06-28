<?hh

class D<Ta, reify Tb> {}
class C<reify Ta, Tb, reify Tc> extends D<int, Tb>{}

function g<T>() :mixed{
  $c = new C<int, T, bool>();
}
<<__EntryPoint>> function main(): void {
g();
}
