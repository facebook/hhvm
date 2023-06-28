<?hh

function a() :mixed{
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

  $result = array_diff_assoc($array1, $array2);
  var_dump($result);
}

function b() :mixed{
  $array1 = varray[0, 1, 2];
  $array2 = varray["00", "01", "2"];
  $result = array_diff_assoc($array1, $array2);
  var_dump($result);
}


<<__EntryPoint>>
function main_array_diff_assoc() :mixed{
a();
b();
}
