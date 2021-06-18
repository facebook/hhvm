<?hh

function my_handler($errno, $errstr, $file, $line) {
  throw new Exception($errstr);
}

function try_takes_null($x) {
  try {
    takes_null($x);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}

function takes_null(null $x) {
  var_dump($x);
}

function main() {
  try_takes_null(null);
  try_takes_null(42);
  try_takes_null(3.14);
  try_takes_null('abc');
  try_takes_null(true);
  try_takes_null(false);
  try_takes_null(new stdClass());
}
<<__EntryPoint>> function main_entry(): void {
set_error_handler(my_handler<>);
main();
}
