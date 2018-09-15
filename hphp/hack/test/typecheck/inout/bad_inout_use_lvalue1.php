<?hh // strict

function f(inout int $i): void {}

function test(): void {
  f(inout 42);
}
