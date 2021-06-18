<?hh

function my_handler($errno, $errstr, $file, $line) {
  throw new Exception($errstr);
}

function try_takes_nothing($x) {
  try {
    takes_nothing($x);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}

function takes_nothing(nothing $x) {
  var_dump($x);
}

function main() {
  try_takes_nothing(42);
  try_takes_nothing(3.14);
  try_takes_nothing('abc');
  try_takes_nothing(true);
  try_takes_nothing(false);
  try_takes_nothing(new stdClass());
  try_takes_nothing(null);
}
<<__EntryPoint>> function main_entry(): void {
set_error_handler(my_handler<>);
main();
}
