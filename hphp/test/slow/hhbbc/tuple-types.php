<?hh


function foo(bool $x, int $n, string $y) :mixed{
  if ($x) {
    $y = tuple($n);
  } else {
    $y = tuple($y);
    echo $y[0];
  }
  return $y[0];
}

<<__EntryPoint>>
function main_tuple_types() :mixed{
var_dump(foo(false, 4, 'hi'));
}
