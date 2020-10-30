<?hh

function returns2<T>(): int {
  return 2;
}

<<__EntryPoint>>
function test(): void {
  // At one point, this parsed as (2 * (foo<int>() + 3))
  // And shared a parsing path with function pointers
  var_dump(2 * returns2<int>() + 3);

  var_dump(5000 * returns2<int>() < 2 + 1);
}
