<?hh // partial

function get_mixed(): mixed {
  return vec[];
}

function get_container(): Container<string> {
  return vec[];
}

function f(): void {
  expect_vec1(get_mixed() as vec); // ok
  expect_vec2(get_mixed() as vec); // error

  expect_vec1(get_mixed() as vec<_>); // ok
  expect_vec2(get_mixed() as vec<_>); // error

  expect_vec1(get_container() as vec<_>); // ok
  expect_vec2(get_container() as vec<_>); // ok
}

function expect_vec1(vec<mixed> $vec): void {}
function expect_vec2(vec<string> $vec): void {}
