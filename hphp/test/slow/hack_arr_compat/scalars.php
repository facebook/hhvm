<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_cmp() :mixed{
  var_dump(darray(vec[1, 2, 3]) === vec[1, 2, 3]);
  var_dump(darray(vec[1, 2, 3]) !== vec[1, 2, 3]);
  var_dump(darray(vec[1, 2, 3]) == vec[1, 2, 3]);
  var_dump(darray(vec[1, 2, 3]) != vec[1, 2, 3]);
  wrap(() ==> darray(vec[1, 2, 3]) < true, __LINE__);
  wrap(() ==> darray(vec[1, 2, 3]) <= true, __LINE__);
  wrap(() ==> darray(vec[1, 2, 3]) > true, __LINE__);
  wrap(() ==> darray(vec[1, 2, 3]) >= true, __LINE__);
  wrap(() ==> darray(vec[1, 2, 3]) <=> true, __LINE__);

  var_dump(vec[1, 2, 3] === darray(vec[1, 2, 3]));
  var_dump(vec[1, 2, 3] !== darray(vec[1, 2, 3]));
  var_dump(vec[1, 2, 3] == darray(vec[1, 2, 3]));
  var_dump(vec[1, 2, 3] != darray(vec[1, 2, 3]));
  wrap(() ==> true < darray(vec[1, 2, 3]), __LINE__);
  wrap(() ==> true <= darray(vec[1, 2, 3]), __LINE__);
  wrap(() ==> true > darray(vec[1, 2, 3]), __LINE__);
  wrap(() ==> true >= darray(vec[1, 2, 3]), __LINE__);
  wrap(() ==> true <=> darray(vec[1, 2, 3]), __LINE__);
}

function test_intlike_keys() :mixed{
  var_dump(darray(dict['0' => 1]));
  var_dump(darray(dict['0' => 1, 1 => 2, 2 => 3, 3 => 4]));
  var_dump(darray(dict[0 => 1, 1 => 2, '2' => 3, 3 => 4]));
  var_dump(darray(dict[0 => 1, 1 => 2, 2 => 3, '3' => 4]));

  var_dump(darray(dict['10' => 10]));
  var_dump(darray(dict['10' => 10, 20 => 20, 30 => 30]));
  var_dump(darray(dict[10 => 10, '20' => 20, 30 => 30]));
  var_dump(darray(dict[10 => 10, 20 => 20, '30' => 30]));
}

function wrap($cmp, $lineno) :mixed{
  echo "\n";
  try {
    $ret = $cmp();
  } catch (InvalidOperationException $e) {
    echo "Caught: ".$e->getMessage()." on line $lineno\n";
    return;
  }
  echo "FAIL ".var_export($ret, true)."\n";
}

<<__EntryPoint>>
function main_scalars() :mixed{
  test_cmp();
  test_intlike_keys();
}
