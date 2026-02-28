<?hh

class C<<<__Warn>> reify T> {}
class D<reify T> {}

function f<reify T>(T $x) :mixed{}

<<__EntryPoint>>
function main(): void {
  // Warn
  f<D<shape('a' => C<int>, 'b' => string)>>(new D<shape('a' => C<string>, 'b' => string)>());
  // Error because second one errors despite the first one should warn
  f<D<shape('a' => C<int>, 'b' => int)>>(new D<shape('a' => C<string>, 'b' => string)>());
}
