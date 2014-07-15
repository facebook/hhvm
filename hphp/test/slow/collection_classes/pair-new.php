<?hh
function main() {
  try {
    var_dump(new Pair());
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump(new Pair(1, 1));
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}
main();
