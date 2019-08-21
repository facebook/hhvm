<?hh
<<__EntryPoint>>
function main(): void {
  var_dump(array_map(fun('get_class'), array(null)));
  var_dump(array_map(fun('get_parent_class'), array(null)));
}
