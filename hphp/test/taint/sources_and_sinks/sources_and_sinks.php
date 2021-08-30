<?hh

function __source_returned_from_function(): int { return 1; }

function __sink(int $data): void {}

function __sink_in_second_parameter(int $first, int $second): void {}

function source_returned_from_function(): void {
  $data = __source_returned_from_function();
  __sink($data);
}

class MyClass {
  public function __source_returned_from_method(): int { return 1; }
}

function source_returned_from_method(): void {
  $object = new MyClass();
  $data = $object->__source_returned_from_method();
  __sink($data);
}

function source_to_sink_in_second_parameter(): void {
  $data = __source_returned_from_function();
  __sink_in_second_parameter(0, $data);
}

function source_to_sink_not_in_second_parameter(): void {
  $data = __source_returned_from_function();
  __sink_in_second_parameter($data, 0);
}

<<__EntryPoint>> function main(): void {
  source_returned_from_function();
  source_returned_from_method();
  source_to_sink_in_second_parameter();
  source_to_sink_not_in_second_parameter();
}
