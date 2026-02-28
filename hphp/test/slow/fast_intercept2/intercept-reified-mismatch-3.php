<?hh

function handler($name, $obj, $args) :mixed{
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args);
  echo "---------------\n";
  return shape('prepend_this' => true, 'callback' => 'bar');
}

function bar<reify T>($_this, $arg) :mixed{
  echo "In bar!\n";
  echo "Arg is: " . $arg . "\n";
  return 7;
}

function foo<reify Ta, reify Tb>($arg) :mixed{
  echo "In foo!\n";
  echo "Arg is: " . $arg . "\n";
  return 5;
}

<<__EntryPoint>>
function main() :mixed{
  fb_intercept2('foo', handler<>);
  var_dump(foo("Hey!"));
}
