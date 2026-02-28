<?hh

<<file:__EnableUnstableFeatures('class_type')>>

class C {}

<<__EntryPoint>>
function f(): void {
  nameof C is class<mixed>; // banned by hack
}
