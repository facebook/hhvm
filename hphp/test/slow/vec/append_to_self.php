<?hh

function append_to_self(): vec {
  $v = vec[];
  for ($i = 0; $i < 4; $i++) {
    $v[] = $v;
  }
  return $v;
}

function main() :mixed{
  echo "---- profiling ----\n";
  var_dump(append_to_self());
  append_to_self();
  echo "---- pgo ----\n";
  var_dump(append_to_self());
}


<<__EntryPoint>>
function main_append_to_self() :mixed{
main();
}
