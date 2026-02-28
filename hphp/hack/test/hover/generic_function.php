<?hh

function g<T>(T $x): void {}

function f(): void {
  g(0);
//^ hover-at-caret
}
