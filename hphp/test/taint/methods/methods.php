<?hh

function __source(): int {
  return 1;
}

function __sink(int $input): void {}

class MyClass {
  public function identity(int $input): int {
    return $input;
  }

  public static function static_identity(int $input): int {
    return $input;
  }

  public function stop_flow(int $input): int {
    return 1;
  }

  public function class_sink(int $input): void {}

  public function class_sink2(int $input): void {}
}

class MySubClass extends MyClass {
  <<__Override>>
  public function identity(int $input): int {
    return $input;
  }

  <<__Override>>
  public function class_sink(int $input): void {}

  <<__Override>>
  public function class_sink2(int $input): void {}
}

class MyBrokenSubClass extends MyClass {
  <<__Override>>
  public function identity(int $input): int {
    // Buggy identity.
    return 1;
  }
}

function source_bypassing_identity_method_into_sink() {
  $object = new MyClass();
  $data = __source();
  $result = $object->identity($data);
  __sink($data);  // Not using $result.
}

function source_through_identity_method_into_sink() {
  $object = new MyClass();
  $data = __source();
  $result = $object->identity($data);
  __sink($result);
}

function source_stopped_in_method() {
  $object = new MyClass();
  $data = __source();
  $result = $object->stop_flow($data);
  __sink($result);
}

function source_through_virtual_identity_method_into_sink(MyClass $object) {
  $data = __source();
  $result = $object->identity($data);
  __sink($result);
}

function source_stopped_in_broken_virtual_identity_method(MyClass $object) {
  $data = __source();
  $result = $object->identity($data);
  __sink($result);
}

function source_through_static_identity_method_into_sink() {
  $data = __source();
  $result = MyClass::static_identity($data);
  __sink($result);
}

function source_to_class_sink() {
  // Verify that Parent::bar being a sink does not mean
  // Child::bar is a sink if the child reimplements it
  $data = __source();
  $object = new MyClass();
  $child = new MySubClass();
  $object->class_sink($data);
  $child->class_sink($data);
  // Verify that specifying a sink on the parent class is sufficient
  // Also verify that the resulting name in the trace is from the parent
  // unless the child reimplements it
  $child2 = new MyBrokenSubClass();
  $object->class_sink2($data);
  $child->class_sink2($data);
  $child2->class_sink2($data);
}

<<__EntryPoint>> function main(): void {
  source_bypassing_identity_method_into_sink();
  source_through_identity_method_into_sink();
  source_stopped_in_method();
  source_through_virtual_identity_method_into_sink(new MySubClass());
  source_stopped_in_broken_virtual_identity_method(new MyBrokenSubClass());
  source_through_static_identity_method_into_sink();
  source_to_class_sink();
}
