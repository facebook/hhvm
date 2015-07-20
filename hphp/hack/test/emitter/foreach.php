<?hh // strict

function foreach_loop_1(array<int> $arr): int {
  $sum = 0;
  foreach ($arr as $x) {
    $sum += $x;
  }
  return $sum;
}

function foreach_loop_2(array<int> $arr): int {
  $sum = 0;
  foreach ($arr as $x) {
    $sum += $x;
    if ($x == -1) break;
    if ($x == -2) continue;
    if ($x == -3) return 42;
  }
  return $sum;
}

function foreach_loop_3(array<int> $arr): int {
  $sum = 0;
  foreach ($arr as $x) {
    try {
      $sum += $x;
      if ($x == -1) break;
      if ($x == -2) continue;
      if ($x == -3) return 42;
    } finally {
      echo "looped\n";
    }
  }
  return $sum;
}


function foreach_loop_kv_1(array<string, int> $arr): int {
  $sum = 0;
  $lol = array();
  foreach ($arr as $k => $x) {
    echo $k, "\n";
    $sum += $x;
  }
  return $sum;
}

function test(): void {
  $test_vecs = array(
    array(1,2,3,4),
    array(1,2,-2,3,4),
    array(1,2,-1,3,4),
    array(1,2,-3,3,4)
  );


  foreach ($test_vecs as $test) {
    var_dump(foreach_loop_1($test));
    var_dump(foreach_loop_2($test));
    var_dump(foreach_loop_3($test));
  }

  $test_vecs = array(
    array('a' => 1, 'b' => 2, 'c' => 3, 'd' => 4),
  );
  foreach ($test_vecs as $test) {
    var_dump(foreach_loop_kv_1($test));
  }
}
