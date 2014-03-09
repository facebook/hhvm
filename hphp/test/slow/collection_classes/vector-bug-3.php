<?hh
function main() {
  Vector::slice(Vector {1, 2}, 1, PHP_INT_MAX);
  echo "Done\n";
}
main();

