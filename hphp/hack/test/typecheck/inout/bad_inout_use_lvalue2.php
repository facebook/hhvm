<?hh

const SOME_CONST = 'FOO';

function f(inout string $i): void {}

function test(): void {
  f(inout SOME_CONST);
}
