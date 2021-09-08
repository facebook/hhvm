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

<<__EntryPoint>> function main(): void {
  source_through_attribute_into_sink();
  source_through_attribute_dereferenced_in_callee();
}
