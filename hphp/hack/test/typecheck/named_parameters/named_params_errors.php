<?hh

function error_test(int $pos, named string $label): void {}

function test_errors(): void {
  // Error: Wrong type for named parameter
  error_test(label=42, 1);

  // Error: Wrong type for positional parameter
  error_test(label="test", "not_int");

  // Error: Missing required positional parameter
  error_test(label="test");

  // Error: Unknown named parameter
  error_test(unknown="test", 42);
}
