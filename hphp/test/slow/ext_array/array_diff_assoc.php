<?hh

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

  $result = array_diff_assoc($array1, $array2);
  var_dump($result);
}

function b() :mixed{
  $array1 = vec[0, 1, 2];
  $array2 = vec["00", "01", "2"];
  $result = array_diff_assoc($array1, $array2);
  var_dump($result);
}


<<__EntryPoint>>
function main_array_diff_assoc() :mixed{
a();
b();
}
