<?hh
<<file: __EnableUnstableFeatures("function_type_optional_params")>>

function foo(int $x, string $y = 'a', bool $z = false): void {}
// It's legal to have optional arguments followed by a variadic
function bar(int $x, string $y = 'a', bool ...$v): void {}

function expect1((function(int, string, bool): void) $_): void {}
function expect2((function(int): void) $_): void {}
function expect3(
  (function(int, optional string, optional bool): void) $f,
): void {
  $f(3);
  $f(4, 'a');
  $f(5, 'b', true);
  // Should be an error
  $f(3, false);
  // Should be an error
  $f(3, 'b', true, 2);
}
function expect4((function(int, optional string, bool...): void) $f): void {
  $f(3);
  $f(4, 'a');
  $f(5, 'b', true);
  $f(6, 'c', true, false);
}

function too_few(int $x): void {}
function too_many(int $x, string $y, bool $b): void {}

function testit(): void {
  $f = foo<>;
  expect1($f);
  expect2($f);
  expect3($f);
  expect4(bar<>);
  // Should be an error
  expect3(too_few<>);
  // Should be an error
  expect3(too_many<>);
}

// Should be able to use optional as an identifier
function optional(): void {}
function test_call(): void {
  optional();
}

// All should be rejected
function badhint((function(int, optional bool, string): void) $_): void {}
function badhint2(
  (function(int, optional bool, optional float, string): void) $_,
): void {}
function badhint3((function(int, optional inout bool): void) $_): void {}
