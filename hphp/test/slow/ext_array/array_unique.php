<?hh

function a() {
  $input = darray[
    "a" => "green",
    0 => "red",
    "b" => "green",
    1 => "blue",
    2 => "red"
  ];
  $result = array_unique($input);
  var_dump($result);
}

function b() {
  $input = varray[
    4,
    "4",
    "3",
    4,
    3
  ];
  $result = array_unique($input);
  var_dump($result);
}

function c() {
  $input = darray[
    "a" => "A",
    "b" => "C",
    0 => "1",
    2 => "01",
    1 => 1,
    "c" => "C"
  ];
  var_dump(array_unique($input, SORT_STRING));
  var_dump(array_unique($input, SORT_NUMERIC));
  var_dump(array_unique($input, SORT_REGULAR));
}

function d() {
  $input = darray[
    1 => 1,
    'a' => 'A',
    'b' => 'C',
    0 => '1',
    2 => '01',
    'c' => 'C'
  ];
  var_dump(array_unique($input, SORT_REGULAR));
}


<<__EntryPoint>>
function main_array_unique() {
a();
b();
c();
d();
}
