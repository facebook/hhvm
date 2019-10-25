<?hh

function handler($name, $obj, inout $args) {
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args);
  echo "---------------\n";
  return shape('prepend_this' => true, 'callback' => 'bar');
}

function bar<reify Ta>($_this, $arg) {
  echo "In bar!\n";
  echo "Arg is: " . $arg . "\n";
  var_dump(HH\ReifiedGenerics\get_type_structure<Ta>());
  return 7;
}

function foo<reify Ta, reify Tb>($arg) {
  echo "In foo!\n";
  echo "Arg is: " . $arg . "\n";
  return 5;
}

<<__EntryPoint>>
function main() {
  fb_intercept2('foo', 'handler');
  var_dump(foo<int, string>("Hey!"));
}
