<?hh

function append_to_self(): array {
  $a = varray[];
  $a[] = $a;
  $a[] = $a;
  $a[] = $a;
  $a[] = $a;
  return $a;
}

function main() {
  echo "---- profiling ----\n";
  var_dump(append_to_self());
  append_to_self();
  echo "---- pgo ----\n";
  var_dump(append_to_self());
}


<<__EntryPoint>>
function main_append_to_self() {
main();
}
