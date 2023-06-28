<?hh


function f($x) :mixed{
  if (($x & 15) != 0) {
    $x  = $x + 3;
    if ($x == 991919188238838) {
      return $x;
    }
  }
  $x++;
  return $x;
}


<<__EntryPoint>>
function main_branchover() :mixed{
for ($i = 0; $i < 20; $i++) {
  $y = f($i);
  var_dump($y);
}
}
