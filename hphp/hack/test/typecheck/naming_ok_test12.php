<?hh // strict

const string X = '';

class X {}

function X(string $x): void {}

function test(): void {
  X(X);
  new X();
}
