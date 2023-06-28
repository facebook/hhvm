<?hh
function main() :mixed{
  $x = Vector {};
  $x->toImmVector();
  $x[] = 1;
  echo "Done\n";
}

<<__EntryPoint>>
function main_vector_bug_2() :mixed{
main();
}
