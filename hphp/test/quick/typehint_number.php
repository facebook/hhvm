<?hh

function error_handler($errno, $errstr, $file, $line) {
  echo $errstr, ' on line ', $line, '; ignoring ...', "\n";
  return true; // don't fatal
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
  takes_num(10);
  takes_num(10.0);
  takes_num(10.5);
  takes_num('10.5'); // nope: no strings, even numeric
  takes_num('foo');  // nope: string
  takes_num(new NumericallyStringable()); // nope: object
  takes_num(new StdClass()); // nope: object
}

set_error_handler('error_handler');
main();
