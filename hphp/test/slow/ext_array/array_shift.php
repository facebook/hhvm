<?hh

function a() :mixed{
  $input = dict["a" => 1, "b" => 2];
  array_shift(inout $input);
  var_dump($input);
}

function b() :mixed{
  $input = dict["a" => 1, 23 => 2];
  array_shift(inout $input);
  var_dump($input);
}

function c() :mixed{
  $input = dict["a" => 1, -23 => 2];
  array_shift(inout $input);
  var_dump($input);
}

function d() :mixed{
  $input = vec["orange", "banana", "apple", "raspberry"];
  $fruit = array_shift(inout $input);
  var_dump($input);
  var_dump($fruit);
}


<<__EntryPoint>>
function main_array_shift() :mixed{
a();
b();
c();
d();
}
