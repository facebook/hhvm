<?hh

class Foo {
  const varray<int> bar = \HH\array_mark_legacy(vec[1, 2, 3]);
  const varray<int> baz = HH\array_mark_legacy(vec[1, 2, 3]);
}

<<__EntryPoint>>
function main(): void {
  var_dump(HH\is_array_marked_legacy(Foo::baz));
}
