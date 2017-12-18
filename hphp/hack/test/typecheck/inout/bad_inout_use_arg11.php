<?hh // strict

function f(inout string $s): void {}

function test(): void {
  $x = dict['foo' => vec['bar']];
  f(inout $x['foo'][]);
}
