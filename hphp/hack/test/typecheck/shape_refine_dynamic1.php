<?hh

function getShape(): ?shape('a' => int, 'b' => string) {
  return null;
}

function testchau(): void {
  $s = getShape() ?? shape();
  if (Shapes::keyExists($s, 'b')) {
    // stuff
  }
  $a = Shapes::idx($s, 'a');
  expect_nullable_int_chau($a);
}

function expect_nullable_int_chau(?int $_): void {}
