<?hh // partial

function get_mixed(): mixed {
  return dict[];
}

function get_container(): KeyedContainer<int, string> {
  return dict[];
}

function f(): void {
  expect_dict1(get_mixed() as dict); // ok
  expect_dict2(get_mixed() as dict); // error

  expect_dict1(get_mixed() as dict<_, _>); // ok
  expect_dict2(get_mixed() as dict<_, _>); // error

  expect_dict1(get_container() as dict<_, _>); // ok
  expect_dict2(get_container() as dict<_, _>); // ok
}

function expect_dict1(dict<arraykey, mixed> $dict): void {}
function expect_dict2(dict<int, string> $dict): void {}
