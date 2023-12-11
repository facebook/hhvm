<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() :mixed{
  var_dump(array_reverse(vec[vec[1], vec[2], vec[3], vec[4]]));
  var_dump(array_reverse(vec[dict[1 => 'a'], dict[2 => 'b'], dict[3 => 'c']]));
  var_dump(array_reverse(vec[keyset['a'], keyset['b'], keyset['c']]));
}

<<__EntryPoint>>
function main_hack_arrays_hphpc_const_fold() :mixed{
main();
}
