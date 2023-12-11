<?hh

function f() :mixed{
  $x = 204; // 11001100 in binary
  $y = 170; // 10101010 in binary
  echo ($x ^ $y); // 01100110 in binary
  echo "\n";
  echo ($x & $y);
  echo "\n";
  echo ($x | $y);
  echo "\n";
  echo (~$x);
  echo "\n";
}

// Pairwise probe.
function probe($l, $r) :mixed{
  echo "-------\n";
  echo "left: ";  var_dump($l);
  echo "right: "; var_dump($r);
  $orig_l = $l;
  if(!($l is string && $r is string)) {
    $l = (int)$l;
    $r = (int)$r;
  }
  $v = ($l & $r); var_dump($v);
  $v = ($l | $r); var_dump($v);
  $v = ($l ^ $r); var_dump($v);
  $v = ~($orig_l is string ? $orig_l : (int)$orig_l); var_dump($v);
}

<<__EntryPoint>>
function main() :mixed{
  f();
  $i = 0x3;
  $data = vec[15, "7", "not an int. at all."];
  foreach ($data as $left) {
    foreach ($data as $right) {
      probe($left, $right);
    }
  }
}
