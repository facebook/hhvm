<?hh

function __source(): int {
  return 1;
}

function __sink(mixed $input): void {}

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

function returns_source(): int {
  return __source();
}

function into_sink(int $data): void {
  __sink($data);
}

function source_through_indirection_to_sink(): void {
  $data = returns_source();
  into_sink($data);
}

function source_through_mutations_into_sink(): void {
  $v = __source();
  $v += 1;
  // Tainted
  __sink($v);
  unset($v);
  $v = 1;
  // Not tainted
  __sink($v);
  $v = __source();
  // Tainted
  __sink($v++);
}

function source_through_global_into_sink(): void {
  $foo = HH\global_get("foo");
  // Not tainted
  __sink($foo);
  HH\global_set("foo", __source());
  $foo = HH\global_get("foo");
  // Tainted
  __sink($foo);
  HH\global_unset("foo");
  $foo = HH\global_get("foo");
  // Not tainted
  __sink($foo);
}

<<__EntryPoint>> function main(): void {
  source_through_assignment_to_sink();
  // Ensure parameters don't pick up taint from prior locals on the stack
  into_sink(0);
  source_through_function_to_sink();
  source_stopped();
  source_through_indirection_to_sink();
  source_through_mutations_into_sink();
  source_through_global_into_sink();
}
