<?hh

function f(inout string $s): void {
  g(inout $s);
}

function g(inout arraykey $a): void {}
