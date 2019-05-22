<?hh

function one() {
  return 1;
}

function main() {
  var_dump(one() % 0);
}

<<__EntryPoint>> function main_entry(): void {
try {
  main();
} catch (DivisionByZeroException $e) {
  echo $e->getMessage(), "\n";
}
}
