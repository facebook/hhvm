<?hh

function __source(): int { return 1; }
function __sink(int $input): void {}

class MyClass {
  public int $attribute;
}

function source_through_attribute_into_sink(): void {
  $object = new MyClass();
  $object->attribute = __source();
  __sink($object->attribute);
}

function into_sink(MyClass $object): void {
  __sink($object->attribute);
}

function source_through_attribute_dereferenced_in_callee(): void {
  $object = new MyClass();
  $object->attribute = __source();
  into_sink($object);
}

function __sink_with_shape(shape('data' => int) $input): void {}

function source_through_shape_into_sink(): void {
  $shape = shape('data' => __source());
  __sink_with_shape($shape);
}

<<__EntryPoint>> function main(): void {
  source_through_attribute_into_sink();
  source_through_attribute_dereferenced_in_callee();
  source_through_shape_into_sink();
}
