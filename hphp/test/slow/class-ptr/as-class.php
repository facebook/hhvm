<?hh

<<file:__EnableUnstableFeatures('class_type')>>

<<__EntryPoint>>
function f(): void {
  3 as class<mixed>; // banned by hack
}
