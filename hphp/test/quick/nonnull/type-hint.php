<?hh

function my_handler($errno, $errstr, $file, $line) {
  throw new Exception($errstr);
}

function try_takes_nonnull($x) {
  try {
    takes_nonnull($x);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}

function takes_nonnull(nonnull $x) {
  var_dump($x);
}

function main() {
  try_takes_nonnull(42);
  try_takes_nonnull(3.14);
  try_takes_nonnull('abc');
  try_takes_nonnull(true);
  try_takes_nonnull(false);
  try_takes_nonnull(new stdClass());
  try_takes_nonnull(null); // nope: null
}
<<__EntryPoint>> function main_entry(): void {
set_error_handler(my_handler<>);
main();
}
