<?hh // strict

function f(inout int $i): void {}

function test(ConstMap<string, shape('eggs' => int)> $x): void {
  f(inout $x['foo']['eggs']);
}
