<?hh

function __source(): int { return 1; }
function __sink(int $input): void {}

class MyClass {
  public int $attribute;
  public static int $static;
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

function objects_of_same_class_are_not_mixed_up(): void {
  $foo = new MyClass();
  $foo->attribute = __source();
  $bar = new MyClass();
  $bar->attribute = 1;
  // This should be flagged
  __sink($foo->attribute);
  // This should not
  __sink($bar->attribute);
  $bar->attribute = $foo->attribute;
  $foo->attribute = 1;
  // This should be flagged
  __sink($bar->attribute);
  // This should not
  __sink($foo->attribute);
}

function object_reassignment_propagates_taint(): void {
  $foo = new MyClass();
  $foo->attribute = __source();
  $bar = new MyClass();
  $bar->attribute = 1;
  $bar = $foo;
  // This should be flagged
  __sink($bar->attribute);
}

function objects_are_properly_tracked_as_shallow_copies(): void {
  $foo = new MyClass();
  $foo->attribute = 1;
  $bar = $foo;
  // Neither should be flagged
  __sink($foo->attribute);
  __sink($bar->attribute);
  $foo->attribute = __source();
  // Both should be flagged
  __sink($foo->attribute);
  __sink($bar->attribute);
}

function source_through_static_into_sink(): void {
  MyClass::$static = __source();
  // This is a valid flow
  __sink(MyClass::$static);
  MyClass::$static = 1;
  // This is not
  __sink(MyClass::$static);
  MyClass::$static += __source();
  // This is
  __sink(MyClass::$static);
}

class MySecondClass {
  public int $i;

  public function __construct(
    int $i
  ) {
    $this->i = $i;
  }
}

function source_through_implemented_constructor_into_sink(): void {
  $foo = new MySecondClass(__source());
  __sink($foo->i);
}

class MyThirdClass {
  public function __construct(
    public int $i
  ) {}
}

function source_through_promoted_constructor_into_sink(): void {
  $foo = new MyThirdClass(__source());
  __sink($foo->i);
}

<<__EntryPoint>> function main(): void {
  source_through_attribute_into_sink();
  source_through_attribute_dereferenced_in_callee();
  source_through_shape_into_sink();
  objects_of_same_class_are_not_mixed_up();
  object_reassignment_propagates_taint();
  objects_are_properly_tracked_as_shallow_copies();
  source_through_static_into_sink();
  source_through_implemented_constructor_into_sink();
  source_through_promoted_constructor_into_sink();
}
