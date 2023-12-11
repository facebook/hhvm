<?hh

function f(mixed $x) :mixed{
  $x__str = (string)($x);
  echo "\$x is $x__str\n";
  try {
    $x as string;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $x as arraykey;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}



<<__EntryPoint>>
function main_primitives() :mixed{
$arr = vec[1, 2, 3, 3.14, Vector {}, "hiii"];

foreach ($arr as $e) {
  f($e);
}
}
