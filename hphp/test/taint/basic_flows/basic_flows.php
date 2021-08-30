<?hh

function __source(): int {
  return 1;
}

function __sink($input): void {}

function identity(int $input, bool $flag): int {
  return $input;
}

function stop(int $input): int {
  return 1;
}

function source_through_assignment_to_sink(): void {
  $data = __source();
  $temporary = $data;
  __sink($temporary);
}

function source_through_function_to_sink(): void {
  $data = __source();
  $temporary = identity($data, /* flag */ false);
  __sink($temporary);
}

function source_stopped(): void {
  $data = __source();
  $temporary = stop($data);
  __sink($temporary);
}

<<__EntryPoint>> function main(): void {
  source_through_assignment_to_sink();
  source_through_function_to_sink();
}
