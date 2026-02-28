<?hh

function test_splat_arity(): void {
  $tuple2 = tuple(1, 2);

  // OK
  expect_const2(...$tuple2);
  expect_const3(0, ...$tuple2);
  expect_variadic0(...$tuple2);
  expect_variadic1(...$tuple2);
  expect_variadic2(...$tuple2);

  // Errors
  expect_const1(...$tuple2);
  expect_const2(0, ...$tuple2);
  expect_variadic3(...$tuple2);

  $vec2 = vec[1, 2];

  // OK
  expect_variadic0(...$vec2);
  expect_variadic1(0, ...$vec2);
  expect_variadic2(0, 1, ...$vec2);
  expect_variadic3(0, 1, 2, ...$vec2);

  // Errors
  expect_const1(...$vec2);
  expect_const1(0, ...$vec2);
  expect_const2(...$vec2);
  expect_const2(0, ...$vec2);
  expect_const2(0, 1, ...$vec2);
}

type M = mixed;

function expect_const1(M $a): void {}
function expect_const2(M $a, M $b): void {}
function expect_const3(M $a, M $b, M $c): void {}
function expect_variadic0(M ...$as): void {}
function expect_variadic1(M $a, M ...$bs): void {}
function expect_variadic2(M $a, M $b, M ...$cs): void {}
function expect_variadic3(M $a, M $b, M $c, M...$ds): void {}
