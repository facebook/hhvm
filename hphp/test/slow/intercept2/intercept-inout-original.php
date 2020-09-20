<?hh

function handler($name, $obj, inout $args) {
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args);
  echo "---------------\n";
  return shape();
}

function foo($arg, inout $a) {
  echo "In foo!\n";
  echo "Arg is: " . $arg . "\n";
  $a = 10;
  return 5;
}

<<__EntryPoint>>
function main() {
  fb_intercept2('foo', 'handler');
  $a = 1;
  var_dump(foo("Hey!", inout $a));
  var_dump($a);
}
