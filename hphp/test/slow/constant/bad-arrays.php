<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

const TESTS = vec[
  'const BADARRAY = [$a];',
  'const BADARRAY = [100 + $a + 200];',
  'const BADARRAY = [new stdClass];',
  'const BADARRAY = [foobaz()];',
  'const BADARRAY = [1, [2, new stdClass], 3];',
  'const BADARRAY = [1, [2, foobaz()], 3];',
  'const BADARRAY = [1, [2, $a], 3];',
  'const BADARRAY = [1, [2, 100 + $a + 200], 3];',

  'const BADVEC = vec[$a];',
  'const BADVEC = vec[100 + $a + 200];',
  'const BADVEC = vec[new stdClass];',
  'const BADVEC = vec[foobaz()];',
  'const BADVEC = vec[1, vec[2, new stdClass], 3];',
  'const BADVEC = vec[1, vec[2, foobaz()], 3];',
  'const BADVEC = vec[1, vec[2, $a], 3];',
  'const BADVEC = vec[1, vec[2, 100 + $a + 200], 3];',

  'const BADDICT = dict[0 => $a];',
  'const BADDICT = dict[100 => 100 + $a + 200];',
  'const BADDICT = dict[1 => new stdClass];',
  'const BADDICT = dict[1 => foobaz()];',
  'const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => new stdClass], 3 => 3];',
  'const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => foobaz()], 3 => 3];',
  'const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => $a], 3 => 33];',
  'const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => 100 + $a + 200], 3 => 3];',

  'const BADKEYSET = keyset[$a];',
  'const BADKEYSET = keyset[100 + $a + 200];',
  'const BADKEYSET = keyset[foobaz()];',
  'const BADKEYSET = keyset[1, keyset[2, foobaz()], 3];',
  'const BADKEYSET = keyset[1, keyset[2, $a], 3];',
  'const BADKEYSET = keyset[1, ketset[2, 100 + $a + 200], 3];'
];

function main() :mixed{
  $count = __hhvm_intrinsics\apc_fetch_no_check('test-count');
  if ($count === false) $count = 0;
  if ($count >= count(TESTS)) return;

  $test = TESTS[$count];
  echo "\"$test\" ====>";
  eval($test);
  echo "\n==============================================================\n";

  $count++;
  apc_store('test-count', $count);
}

<<__EntryPoint>>
function main_bad_arrays() :mixed{
main();
}
