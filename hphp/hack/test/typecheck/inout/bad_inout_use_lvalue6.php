<?hh

function f(inout int $i): void {}

function g(): int {
  $x = 42;
  return $x;
}

function h(): dict<string, int> {
  return dict[];
}

function test(): void {
  f(inout g());
  f(inout h()['foo']);
}
