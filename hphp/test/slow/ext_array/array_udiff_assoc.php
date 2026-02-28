<?hh

function comp_func($n1,$n2) :mixed{
  $n1=(int)$n1; $n2=(int)$n2;
  return $n1 === $n2 ? 0 : ($n1 > $n2 ? 1 : -1);
}


<<__EntryPoint>>
function main_array_udiff_assoc() :mixed{
$a = dict["0.1" => 9, "0.5" => 12, 0 => 23, 1 => 4, 2 => -15];
$b = dict["0.2" => 9, "0.5" => 22, 0 => 3, 1 => 4, 2 => -15];

$result = array_udiff_assoc($a, $b, comp_func<>);
var_dump($result);
}
