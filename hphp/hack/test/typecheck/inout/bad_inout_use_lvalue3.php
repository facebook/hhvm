<?hh // strict

class Cls {
  const Some_CLASS_CONST = 3.14159;
}

function f(inout float $i): void {}

function test(): void {
  f(inout Cls::Some_CLASS_CONST);
}
