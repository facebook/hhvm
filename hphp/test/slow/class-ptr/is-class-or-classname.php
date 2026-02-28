<?hh

class C {}

<<__EntryPoint>>
function f(): void {
  nameof C is class_or_classname<mixed>; // banned by hack
}
