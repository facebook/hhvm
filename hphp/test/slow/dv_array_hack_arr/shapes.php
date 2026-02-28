<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test() :mixed{
  echo "=========================\n";
  $x = tuple();
  var_dump(HH\is_php_array($x));
  var_dump(is_vec($x));
  var_dump(is_dict($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));

  echo "=========================\n";
  $x = tuple('a', 'b', 'c');
  var_dump(HH\is_php_array($x));
  var_dump(is_vec($x));
  var_dump(is_dict($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));

  echo "=========================\n";
  $x = shape();
  var_dump(HH\is_php_array($x));
  var_dump(is_vec($x));
  var_dump(is_dict($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));

  echo "=========================\n";
  $x = shape('a' => 11, 'b' => 22, 'c' => 33);
  var_dump(HH\is_php_array($x));
  var_dump(is_vec($x));
  var_dump(is_dict($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
}


<<__EntryPoint>>
function main_shapes() :mixed{
test();
}
