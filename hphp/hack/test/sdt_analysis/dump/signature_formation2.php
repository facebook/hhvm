<?hh

function g<T>(T $t): void {}

function f<T>(T $t): void {
  g($t);
}
