<?hh

class Info { static bool $sawHackArrNotice = false; }
function handler($_errno, $errstr, ...) {
  if (
    !Info::$sawHackArrNotice &&
    $errstr === 'Hack Array Compat: Comparing PHP array with Hack array'
  ) {
    Info::$sawHackArrNotice = true;
    return true;
  }
  return false;
}

function do_compare_hack_array($cmp) {
  Info::$sawHackArrNotice = false;
  try {
    $cmp();
  } catch (InvalidOperationException $e) {
    return '*';
  }
  return Info::$sawHackArrNotice ? 'T' : 'F';
}
function do_compare_non_any_array($cmp) {
  try {
    $cmp();
  } catch (InvalidOperationException $e) {
    return '*';
  }
  return 'F';
}
function do_compares($a, $b, $cmp) {
  echo "=========================== Notice Compare =======================\n";
  var_dump($a);
  var_dump($b);
  echo $cmp(() ==> $a < $b);
  echo " ".$cmp(() ==> $a <= $b);
  echo " ".$cmp(() ==> $a > $b);
  echo " ".$cmp(() ==> $a >= $b);
  echo " ".$cmp(() ==> $a <=> $b);
  echo " ".$cmp(() ==> $a == $b);
  echo " ".$cmp(() ==> $a != $b);
  echo " ".$cmp(() ==> $a === $b);
  echo " ".$cmp(() ==> $a !== $b);
  echo "\n==================================================================\n";
}

<<__EntryPoint>>
function main_compare() {
  set_error_handler(handler<>);

  $x1 = vec[
    varray[],
    varray[1, 2, varray[3, 4]],
    darray['a' => 'b', 'c' => 'd'],
  ];
  $x2_non_hack_arrays = vec[
    true,
    false,
    null,
    123,
    4.567,
    'abc',
    new stdClass,
    imagecreate(1, 1),
    varray[1, varray[2, 5], varray[3, 4]],
    darray['a' => varray[], 'c' => varray[1, 2]],
  ];
  $x2_hack_arrays = vec[
    vec[],
    vec[1, 2, 3],
    dict[],
    dict['a' => 'b', 'c' => 'd'],
    keyset[],
    keyset['a', 'b', 'c'],
    varray[1, 2, vec[3, 4]],
  ];

  foreach ($x1 as $a) {
    foreach ($x2_non_hack_arrays as $b) {
      do_compares($a, $b, do_compare_non_any_array<>);
      do_compares($b, $a, do_compare_non_any_array<>);
    }
    foreach ($x2_hack_arrays as $b) {
      do_compares($a, $b, do_compare_hack_array<>);
      do_compares($b, $a, do_compare_hack_array<>);
    }
  }

  do_compares(null, null, do_compare_non_any_array<>);
  do_compares(true, false, do_compare_non_any_array<>);
  do_compares(1, 2, do_compare_non_any_array<>);

  do_compares(null, null, do_compare_hack_array<>);
  do_compares(true, false, do_compare_hack_array<>);
  do_compares(1, 2, do_compare_hack_array<>);
}
