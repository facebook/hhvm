<?hh

function handler($name, $obj, $args) {
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args);
  echo "---------------\n";
  return shape('callback' => 'bar$1$inout');
}

function bar($arg, inout $a) {
  echo "In bar!\n";
  echo "Arg is: " . $arg . "\n";
  $a = 7;
}

function foo($arg, inout $a) {
  echo "In foo!\n";
  echo "Arg is: " . $arg . "\n";
  $a = 5;
}

<<__EntryPoint>>
function main() {
  fb_intercept2('foo', 'handler');
  $x = 1;
  foo("Hey!", inout $x);
  var_dump($x);
}
