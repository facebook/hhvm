<?hh

function a() :mixed{
  $input = dict[
    "a" => "green",
    0 => "red",
    "b" => "green",
    1 => "blue",
    2 => "red"
  ];
  $result = array_unique($input);
  var_dump($result);
}

function b() :mixed{
  $input = vec[
    4,
    "4",
    "3",
    4,
    3
  ];
  $result = array_unique($input);
  var_dump($result);
}

function c() :mixed{
  $input = dict[
    "a" => "A",
    "b" => "C",
    0 => "1",
    2 => "01",
    "c" => "C"
  ];
  var_dump(array_unique($input, SORT_STRING));
  var_dump(array_unique($input, SORT_NUMERIC));
  var_dump(array_unique($input, SORT_REGULAR));
}

function d() :mixed{
  $input = dict[
    'a' => 'A',
    'b' => 'C',
    0 => '1',
    2 => '01',
    'c' => 'C'
  ];
  var_dump(array_unique($input, SORT_REGULAR));
}


<<__EntryPoint>>
function main_array_unique() :mixed{
a();
b();
c();
d();
}
