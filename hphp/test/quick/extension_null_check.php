<?hh
<<__EntryPoint>>
function main(): void {
  var_dump(array_map(fun('get_class'), varray[null]));
  var_dump(array_map(fun('get_parent_class'), varray[null]));
}
