<?hh

function f(mixed $x) :mixed{
  try {
    $x as shape('h\'"i' => int, "w\'o\\w" => string);
    echo "\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_shape_escaped_key_errors() :mixed{
$arr = vec[
  shape('h\'"i' => 'wow', "w\'o\\w" => "hi"),
  shape('h\'"i' => 1, "w\'o\\w" => 2),
];

foreach ($arr as $a) {
  f($a);
}
}
