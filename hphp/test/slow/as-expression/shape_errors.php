<?hh

function f(mixed $x) :mixed{
  try {
    $x as shape('hi' => int, 'wow' => string);
    echo "\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function g(mixed $x) :mixed{
  try {
    $x as shape('hi' => int, 'wow' => string, ...);
    echo "OK\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function h(mixed $x) :mixed{
  try {
    $x as shape(?'hi' => int, 'wow' => string);
    echo "OK\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function test($arr, $f) :mixed{
  foreach ($arr as $a) {
    $f($a);
  }
  echo "\n";
}


<<__EntryPoint>>
function main_shape_errors() :mixed{
$arr = vec[
  shape('hi' => 'wow'),
  shape('hi' => 1, 'wow' => 2),
  shape('hi' => 1, 'wow' => 2, 'extra' => 3),
  shape('wow' => "hi"),
];

test($arr, f<>);
test($arr, g<>);
test($arr, h<>);
}
