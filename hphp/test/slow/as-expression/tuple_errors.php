<?hh

function f(mixed $x) :mixed{
  try {
    $x as (int, bool, string);
    echo "OK\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function g(mixed $x) :mixed{
  try {
    $x as (int, (int, (int, bool)));
    echo "OK\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_tuple_errors() :mixed{
$arr = vec[
  tuple(1, true, "hi"),
  tuple(1, 1, "hi"),
  tuple(1, true, 1),
  tuple(1, tuple(1, tuple(1, true))),
  tuple(1, tuple(1, tuple(true, true))),
  tuple(1, tuple("hi", 1)),
  tuple(1, tuple(1, 1, 1, 1), 1, 1),
];

foreach ($arr as $k => $a) {
  f($a);
}
echo "\n";
foreach ($arr as $k => $a) {
  g($a);
}
}
