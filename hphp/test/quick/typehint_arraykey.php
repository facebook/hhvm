<?hh

function my_handler($errno, $errstr, $file, $line) :mixed{
  throw new Exception($errstr);
}

function try_takes_arraykey($a) :mixed{
  try {
    takes_arraykey($a);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}

function takes_arraykey(arraykey $a) :mixed{
  var_dump($a);
}

function main() :mixed{
  try_takes_arraykey(10);
  try_takes_arraykey('abc');
  try_takes_arraykey('100');
  try_takes_arraykey('10.5');
  try_takes_arraykey(10.5); // nope: force explicit cast to int or string
  try_takes_arraykey(true); // nope: force explicit cast to int or string
  try_takes_arraykey(null); // nope: force explicit cast to int or string
  try_takes_arraykey(new stdClass()); // nope: object
}
<<__EntryPoint>> function main_entry(): void {
set_error_handler(my_handler<>);
main();
}
