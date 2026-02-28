<?hh

function f(inout int $i): void {}

function test(shape('eggs' => Map<string, int>) $x): void {
  f(inout $x['eggs']['foo']);
}
