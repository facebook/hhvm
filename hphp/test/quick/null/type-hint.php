<?hh

function my_handler($errno, $errstr, $file, $line) :mixed{
  throw new Exception($errstr);
}

function try_takes_null($x) :mixed{
  try {
    takes_null($x);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}

function takes_null(null $x) :mixed{
  var_dump($x);
}

function main() :mixed{
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
