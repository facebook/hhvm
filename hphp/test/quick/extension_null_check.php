<?hh
<<__EntryPoint>>
function main(): void {
  var_dump(array_map('get_class', array(null)));
  var_dump(array_map('get_parent_class', array(null)));
}
