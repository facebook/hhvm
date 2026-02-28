<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

async function async_with_named(
  int $base,
  named string $message,
): Awaitable<string> {
  return "$message: $base";
}

async function test_async(): HH\Awaitable<void> {
  $result1 = await async_with_named(42, message="test");
  $result2 = await async_with_named(100, message="hello");

  // Error: Missing required named argument: message
  $result3 = await async_with_named(42);

  // Error: Unexpected named argument 'extra'
  $result4 = await async_with_named(42, message="test", extra=123);
}
