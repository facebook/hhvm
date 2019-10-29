<?hh

function simple_function(): void {}
function simple_int_function(): int {}
function simple_function_with_body(): float {
  return 0.5;
}

function function_with_args(int $arg1, float $arg2): void {}

type Typedef = string;
function function_with_non_primitive_args(Typedef $arg1): Typedef {}
