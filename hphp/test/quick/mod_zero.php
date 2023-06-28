<?hh

function one() :mixed{
  return 1;
}

function main() :mixed{
  var_dump(one() % 0);
}

<<__EntryPoint>> function main_entry(): void {
try {
  main();
} catch (DivisionByZeroException $e) {
  echo $e->getMessage(), "\n";
}
}
