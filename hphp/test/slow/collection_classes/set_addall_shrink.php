<?hh
function main() {
  $s = Set {};
  $s->addAll(Vector{1});
  $s->reserve(23);
  $s->addAll(Vector{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1});
  var_dump($s);
}

<<__EntryPoint>>
function main_set_addall_shrink() {
main();
}
