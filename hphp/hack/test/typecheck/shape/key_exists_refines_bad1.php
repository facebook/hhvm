<?hh // strict

function expect_string(string $_): void {}

function test(shape(?'x' => int) $s): void {
  if (Shapes::keyExists($s, 'x')) {
    expect_string($s['x']);
  }
}
