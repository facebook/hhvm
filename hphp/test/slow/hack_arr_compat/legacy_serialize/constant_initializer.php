<?hh

class Foo {
  const varray<int> baz = \HH\array_mark_legacy(varray[1, 2, 3]);
}

<<__EntryPoint>>
function main(): void {
  var_dump(HH\is_array_marked_legacy(Foo::baz));
}
