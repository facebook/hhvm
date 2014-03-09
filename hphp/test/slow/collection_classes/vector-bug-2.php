<?hh
function main() {
  $x = Vector {};
  $x->toImmVector();
  $x[] = 1;
  echo "Done\n";
}
main();
