<?hh

function comp_func($n1,$n2) {
  $n1=(int)$n1; $n2=(int)$n2;
  return $n1 === $n2 ? 0 : ($n1 > $n2 ? 1 : -1);
}


function a() {
  $array1 = darray[
    "a" => "green",
    "b" => "brown",
    "c" => "blue",
    0 => "red"
  ];
  $array2 = darray[
    "a" => "green",
    0 => "yellow",
    1 => "red"
  ];

  $result = array_diff_uassoc($array1, $array2, fun("comp_func"));
  var_dump($result);
}


<<__EntryPoint>>
function main_array_diff_uassoc() {
a();
}
