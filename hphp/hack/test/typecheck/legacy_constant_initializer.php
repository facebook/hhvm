<?hh

class Foo {
  const vec<int> baz = \HH\mark_legacy_hack_array(vec[1, 2, 3]);
}

<<__EntryPoint>>
function main(): void {
  var_dump(HH\is_marked_legacy_hack_array(Foo::baz));
}
