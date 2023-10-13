<?hh

// Non-literal index

function test(string $index): void {
  Shapes::keyExists(shape(), $index);
}
