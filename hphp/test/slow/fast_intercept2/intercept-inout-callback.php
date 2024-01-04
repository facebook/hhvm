<?hh

function handler($name, $obj, inout $args) :mixed{
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args);
  echo "---------------\n";
  return shape('callback' => 'bar');
}

function bar($arg, inout $a) :mixed{
  echo "In bar!\n";
  echo "Arg is: " . $arg . "\n";
  $a = 7;
}

function foo($arg, inout $a) :mixed{
  echo "In foo!\n";
  echo "Arg is: " . $arg . "\n";
  $a = 5;
}

<<__EntryPoint>>
function main() :mixed{
  fb_intercept2('foo', 'handler');
  $x = 1;
  foo("Hey!", inout $x);
  var_dump($x);
}
