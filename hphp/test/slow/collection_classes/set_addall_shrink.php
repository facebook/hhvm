<?hh
function main() {
  $s = Set {};
  $s->addAll(Vector{1});
  $s->reserve(23);
  $s->addAll(Vector{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1});
  var_dump($s);
}
main();
