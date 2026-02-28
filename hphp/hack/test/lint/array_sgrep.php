<?hh

function test() {
  array_merge(FOO);
  array_replace($X, FOO);
  array_push(Xxx, Yyy);
  head_UNTYPED(array_values(Xxx));
  head_UNTYPED(array_keys(Xxx));
  array_create_set_from_values(array_keys(Xxx));
  idx(Xxx, Yyy, null);
}

function test_syntax() {
  $x = X + Y;
  $y = $x + vec[Y];
  $x = array_concat($x, Y);
  $X = array_concat($x, Y);
  $X = array_merge_by_key($X, Y);
}

function test2() {
  first(array_values(Xxx));
  firstx(array_values(Xxx));
  first(array_keys(Xxx));
  firstx(array_keys(Xxx));
}
