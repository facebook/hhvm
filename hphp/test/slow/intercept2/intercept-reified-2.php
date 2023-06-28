<?hh

function handler($name, $obj, inout $args) :mixed{
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args);
  echo "---------------\n";
  return shape('callback' => 'bar');
}

function bar<reify Ta, reify Tb>($arg) :mixed{
  echo "In bar!\n";
  echo "Arg is: " . $arg . "\n";
  var_dump(HH\ReifiedGenerics\get_type_structure<Ta>());
  var_dump(HH\ReifiedGenerics\get_type_structure<Tb>());
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
  var_dump(foo<int, string>("Hey!"));
}
