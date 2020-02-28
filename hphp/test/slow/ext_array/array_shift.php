<?hh

function a() {
  $input = darray["a" => 1, "b" => 2];
  array_shift(inout $input);
  var_dump($input);
}

function b() {
  $input = darray["a" => 1, 23 => 2];
  array_shift(inout $input);
  var_dump($input);
}

function c() {
  $input = darray["a" => 1, -23 => 2];
  array_shift(inout $input);
  var_dump($input);
}

function d() {
  $input = varray["orange", "banana", "apple", "raspberry"];
  $fruit = array_shift(inout $input);
  var_dump($input);
  var_dump($fruit);
}


<<__EntryPoint>>
function main_array_shift() {
a();
b();
c();
d();
}
