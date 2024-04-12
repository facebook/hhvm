<?hh

function expect_string(string $s): void {}
function expect_nstring(?string $s): void {}
function expect_tstring<T as string>(T $s): T { return $s; }
function expect_tnstring<T as ?string>(T $s): T { return $s; }
function expect_ntstring<T as string>(?T $s): T { return $s as nonnull; }
function expect_ntnstring<T as ?string>(?T $s): T { return $s as nonnull; }

class C {}

function error_cases(): void {
  expect_string(C::class);
  expect_nstring(C::class);
}

function error_only_syntactic(): void {
  $c = C::class;
  expect_string($c);
}

function future_cases(): void {
  expect_tstring(C::class);
  expect_tnstring(C::class);
  expect_ntstring(C::class);
  expect_ntnstring(C::class);
}
