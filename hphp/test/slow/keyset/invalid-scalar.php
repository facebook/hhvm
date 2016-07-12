<?hh
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
  $h = keyset[array()];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $i = keyset[new stdclass];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $j = false;
  $k = keyset[$j];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
