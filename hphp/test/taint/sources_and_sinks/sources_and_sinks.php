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

class DataEntity {
  public function getField(): int { return 1; }
}

function source_from_regex_to_sink(): void {
  $entity = new DataEntity();
  __sink($entity->getField());
}

class DataLogger {
  public static async function genLog(int $data): Awaitable<void> {}
}

async function source_to_regex_sink(): Awaitable<void> {
  await DataLogger::genLog(__source_returned_from_function());
}

function parameter_as_source_to_sink(int $data): void {
  __sink($data);
}

<<__EntryPoint>> async function main(): Awaitable<void> {
  source_returned_from_function();
  source_returned_from_method();
  source_to_sink_in_second_parameter();
  source_to_sink_not_in_second_parameter();
  source_from_regex_to_sink();
  await source_to_regex_sink();
  parameter_as_source_to_sink(1);
}
