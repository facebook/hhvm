<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

const TESTS = vec[
  'class Cls1 { static public $a = 123; const BADARRAY = [$a]; };',
  'class Cls1 { static public $a = 123; const BADARRAY = [100 + $a + 200]; };',
  'class Cls1 { const BADARRAY = [new stdClass]; };',
  'class Cls1 { const BADARRAY = [foobaz()]; };',
  'class Cls1 { const BADARRAY = [1, [2, new stdClass], 3]; };',
  'class Cls1 { const BADARRAY = [1, [2, foobaz()], 3]; };',
  'class Cls1 { static public $a = 123; const BADARRAY = [1, [2, $a], 3]; };',
  'class Cls1 { static public $a = 123; const BADARRAY = [1, [2, 100 + $a + 200], 3]; };',

  'class Cls1 { static public $a = 123; const BADVEC = vec[$a]; };',
  'class Cls1 { static public $a = 123; const BADVEC = vec[100 + $a + 200]; };',
  'class Cls1 { const BADVEC = vec[new stdClass]; };',
  'class Cls1 { const BADVEC = vec[foobaz()]; };',
  'class Cls1 { const BADVEC = vec[1, vec[2, new stdClass], 3]; };',
  'class Cls1 { const BADVEC = vec[1, vec[2, foobaz()], 3]; };',
  'class Cls1 { static public $a = 123; const BADVEC = vec[1, vec[2, $a], 3]; };',
  'class Cls1 { static public $a = 123; const BADVEC = vec[1, vec[2, 100 + $a + 200], 3]; };',

  'class Cls1 { static public $a = 123; const BADDICT = dict[0 => $a]; };',
  'class Cls1 { static public $a = 123; const BADDICT = dict[100 => 100 + $a + 200]; };',
  'class Cls1 { const BADDICT = dict[1 => new stdClass]; };',
  'class Cls1 { const BADDICT = dict[1 => foobaz()]; };',
  'class Cls1 { const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => new stdClass], 3 => 3]; };',
  'class Cls1 { const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => foobaz()], 3 => 3]; };',
  'class Cls1 { static public $a = 123; const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => $a], 3 => 33]; };',
  'class Cls1 { static public $a = 123; const BADDICT = dict[1 => 1, 2 => dict[100 => 2, 200 => 100 + $a + 200], 3 => 3]; };',

  'class Cls1 { static public $a = 123; const BADKEYSET = keyset[$a]; };',
  'class Cls1 { static public $a = 123; const BADKEYSET = keyset[100 + $a + 200]; };',
  'class Cls1 { const BADKEYSET = keyset[foobaz()]; };',
  'class Cls1 { const BADKEYSET = keyset[1, keyset[2, foobaz()], 3]; };',
  'class Cls1 { static public $a = 123; const BADKEYSET = keyset[1, keyset[2, $a], 3]; };',
  'class Cls1 { static public $a = 123; const BADKEYSET = keyset[1, ketset[2, 100 + $a + 200], 3]; };'
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
