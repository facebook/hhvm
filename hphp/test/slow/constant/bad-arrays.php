<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

const TESTS = vec[
  '$a = 123; const BADARRAY = [$a];',
  '$a = 123; const BADARRAY = [100 + $a + 200];',
  'const BADARRAY = [new stdclass];',
  'const BADARRAY = [foobaz()];',
  'const BADARRAY = [1, [2, new stdclass], 3];',
  'const BADARRAY = [1, [2, foobaz()], 3];',
  'const BADARRAY = [$GLOBALS];',
  '$a = 123; const BADARRAY = [1, [2, $a], 3];',
  '$a = 123; const BADARRAY = [1, [2, 100 + $a + 200], 3];',

  '$a = 123; const BADVEC = vec[$a];',
  '$a = 123; const BADVEC = vec[100 + $a + 200];',
  'const BADVEC = vec[new stdclass];',
  'const BADVEC = vec[foobaz()];',
  'const BADVEC = vec[1, vec[2, new stdclass], 3];',
  'const BADVEC = vec[1, vec[2, foobaz()], 3];',
  'const BADVEC = vec[$GLOBALS];',
  '$a = 123; const BADVEC = vec[1, vec[2, $a], 3];',
  '$a = 123; const BADVEC = vec[1, vec[2, 100 + $a + 200], 3];',

  '$a = 123; const BADDICT = dict[0 => $a];',
  '$a = 123; const BADDICT = dict[100 => 100 + $a + 200];',
  'const BADDICT = dict[1 => new stdclass];',
  'const BADDICT = dict[1 => foobaz()];',
  'const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => new stdclass], 3 => 3];',
  'const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => foobaz()], 3 => 3];',
  'const BADDICT = dict[1 => $GLOBALS];',
  '$a = 123; const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => $a], 3 => 33];',
  '$a = 123; const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => 100 + $a + 200], 3 => 3];',

  '$a = 123; const BADKEYSET = keyset[$a];',
  '$a = 123; const BADKEYSET = keyset[100 + $a + 200];',
  'const BADKEYSET = keyset[foobaz()];',
  'const BADKEYSET = keyset[1, keyset[2, foobaz()], 3];',
  '$a = 123; const BADKEYSET = keyset[1, keyset[2, $a], 3];',
  '$a = 123; const BADKEYSET = keyset[1, ketset[2, 100 + $a + 200], 3];'
];

function main() {
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
function main_bad_arrays() {
main();
}
