<?hh

function my_handler($errno, $errstr, $file, $line) :mixed{
  throw new Exception($errstr);
}

function try_takes_num($a) :mixed{
  try {
    takes_num($a);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}

function takes_num(num $a) :mixed{
  var_dump($a);
}

class NumericallyStringable {
  function __toString() :mixed{
    return "100";
  }
}

function main() :mixed{
  try_takes_num(10);
  try_takes_num(10.0);
  try_takes_num(10.5);
  try_takes_num('10.5'); // nope: no strings, even numeric
  try_takes_num('foo');  // nope: string
  try_takes_num(new NumericallyStringable()); // nope: object
  try_takes_num(new stdClass()); // nope: object
}
<<__EntryPoint>> function main_entry(): void {
set_error_handler(my_handler<>);
main();
}
