<?hh

enum ShapeKey: string as string {
  A = 'a';
}

type Foo = shape(
  ShapeKey::A => string, // Find ref
);

function test_shape_key(): void {
  shape(ShapeKey::A => '123'); // Find ref
}
