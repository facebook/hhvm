<?hh

function one() {
  return 1;
}

function main() {
  var_dump(one() % 0);
}
try {
  main();
} catch (DivisionByZeroException $e) {
  echo $e->getMessage(), "\n";
}
