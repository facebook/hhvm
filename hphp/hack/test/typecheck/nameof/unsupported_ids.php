<?hh

<<file: __EnableUnstableFeatures('nameof_class')>>

const int C = 4;
function d(): void {}

function f(): void {
  nameof C;
  nameof d;
}
