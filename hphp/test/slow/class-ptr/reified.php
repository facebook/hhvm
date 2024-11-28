<?hh

<<file:__EnableUnstableFeatures('class_type')>>

function f<reify T>(T $t): void {}

<<__EntryPoint>>
function main(): void {
  f<class<C>>(nameof C); // banned by hack
}
