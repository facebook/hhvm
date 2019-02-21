<?hh // partial

function get_mixed(): mixed {
  return keyset[];
}

function get_container(): Container<string> {
  return keyset[];
}

function f(): void {
  expect_keyset1(get_mixed() as keyset); // ok
  expect_keyset2(get_mixed() as keyset); // error

  expect_keyset1(get_mixed() as keyset<_>); // ok
  expect_keyset2(get_mixed() as keyset<_>); // error

  expect_keyset1(get_container() as keyset<_>); // ok
  expect_keyset2(get_container() as keyset<_>); // ok
}

function expect_keyset1(keyset<arraykey> $keyset): void {}
function expect_keyset2(keyset<string> $keyset): void {}
