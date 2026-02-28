<?hh

function comp_func($n1,$n2) :mixed{
  $n1=(int)$n1; $n2=(int)$n2;
  return $n1 === $n2 ? 0 : ($n1 > $n2 ? 1 : -1);
}


function a() :mixed{
  $array1 = dict[
    "a" => "green",
    "b" => "brown",
    "c" => "blue",
    0 => "red"
  ];
  $array2 = dict[
    "a" => "green",
    0 => "yellow",
    1 => "red"
  ];

  $result = array_diff_uassoc($array1, $array2, comp_func<>);
  var_dump($result);
}


<<__EntryPoint>>
function main_array_diff_uassoc() :mixed{
a();
}
