<?hh // strict

function f(inout int $i): void {}

function test(shape('eggs' => ConstMap<string, int>) $x): void {
  f(inout $x['eggs']['foo']);
}
