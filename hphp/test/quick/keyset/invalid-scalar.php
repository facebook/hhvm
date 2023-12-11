<?hh

<<__EntryPoint>> function main(): void {
  try {
    $d = keyset[1.1];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
  try {
    $e = keyset[false];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
  try {
    $f = keyset[true];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
  try {
    $g = keyset[null];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
  try {
    $h = keyset[vec[]];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
  try {
    $i = keyset[new stdClass];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
  try {
    $j = false;
    $k = keyset[$j];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
  try {
    $l = keyset[vec[]];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
  try {
    $m = keyset[dict[]];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
  try {
    $n = keyset[keyset[]];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
  try {
    $o = keyset[Vector{1, 2, 3}];
  } catch (Exception $e) {
    var_dump("Got ".$e->getMessage());
  }
}
