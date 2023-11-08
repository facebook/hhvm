<?hh

<<file: __EnableUnstableFeatures('nameof_class')>>

class C {}
type X = C;
type Y = int;
function f(): void {
  nameof X;
  nameof Y;
}
