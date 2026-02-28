<?hh

class C<<<__Warn>> reify T> {}
class D<reify T> {}

function f<reify T>(T $x) :mixed{}

<<__EntryPoint>>
function main(): void {
  // Warn
  f<D<(C<int>, string)>>(new D<(C<string>, string)>());
  // Error because second one errors despite the first one should warn
  f<D<(C<int>, int)>>(new D<(C<string>, string)>());
}
