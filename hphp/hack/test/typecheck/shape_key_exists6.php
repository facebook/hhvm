<?hh

// Not matching return type of keyExists

function test(): string {
  return Shapes::keyExists(shape('z' => 3), 'z');
}
