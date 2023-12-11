<?hh
<<__EntryPoint>>
function main(): void {
  var_dump(array_map(get_class<>, vec[null]));
  var_dump(array_map(get_parent_class<>, vec[null]));
}
