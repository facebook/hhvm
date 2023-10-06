<?hh

<<file:__EnableUnstableFeatures('nameof_class')>>

class C {}

function f(): string {
  return nameof C;
}
