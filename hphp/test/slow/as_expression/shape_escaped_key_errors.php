<?hh

function f(mixed $x) {
  try {
    $x as shape('h\'"i' => int, "w\'o\\w" => string);
    echo "\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

$arr = vec[
  shape('h\'"i' => 'wow', "w\'o\\w" => "hi"),
  shape('h\'"i' => 1, "w\'o\\w" => 2),
];

foreach ($arr as $a) {
  f($a);
}
