<?hh

function my_handler($errno, $errstr, $file, $line) {
  throw new Exception($errstr);
}

function try_takes_num($a) {
  try {
    takes_num($a);
  } catch (Exception $e) {
    echo $e->getMessage(), "\n";
  }
}

function takes_num(num $a) {
  var_dump($a);
}

class NumericallyStringable {
  function __toString() {
    return "100";
  }
}

function main() {
  try_takes_num(10);
  try_takes_num(10.0);
  try_takes_num(10.5);
  try_takes_num('10.5'); // nope: no strings, even numeric
  try_takes_num('foo');  // nope: string
  try_takes_num(new NumericallyStringable()); // nope: object
  try_takes_num(new StdClass()); // nope: object
}

set_error_handler('my_handler');
main();
