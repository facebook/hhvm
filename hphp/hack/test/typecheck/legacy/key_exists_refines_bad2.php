<?hh // strict

function expect_string(string $_): void {}

function test(shape(...) $s): void {
  if (Shapes::keyExists($s, 'x') && $s['x'] !== null) {
    expect_string($s['x']);
  }
}
