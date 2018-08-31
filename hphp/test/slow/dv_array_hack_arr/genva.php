<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function val() {
  return __hhvm_intrinsics\launder_value(42);
}

async function test_genva() {
  $x = await genva();
  var_dump(is_array($x));
  var_dump(is_vec($x));
  var_dump(is_dict($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));

  $x2 = await genva(val());
  var_dump(is_array($x2));
  var_dump(is_vec($x2));
  var_dump(is_dict($x2));
  var_dump(is_varray($x2));
  var_dump(is_darray($x2));

  $x3 = await genva(val(), val(), val());
  var_dump(is_array($x3));
  var_dump(is_vec($x3));
  var_dump(is_dict($x3));
  var_dump(is_varray($x3));
  var_dump(is_darray($x3));
}

async function test_gena() {
  $x = await gena([]);
  var_dump(is_array($x));
  var_dump(is_vec($x));
  var_dump(is_dict($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));

  $x2 = await gena([val()]);
  var_dump(is_array($x2));
  var_dump(is_vec($x2));
  var_dump(is_dict($x2));
  var_dump(is_varray($x2));
  var_dump(is_darray($x2));

  $x3 = await gena([val(), val(), val()]);
  var_dump(is_array($x3));
  var_dump(is_vec($x3));
  var_dump(is_dict($x3));
  var_dump(is_varray($x3));
  var_dump(is_darray($x3));
}


<<__EntryPoint>>
function main_genva() {
\HH\Asio\join(test_genva());
\HH\Asio\join(test_gena());
}
