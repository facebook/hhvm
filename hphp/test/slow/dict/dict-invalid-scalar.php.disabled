<?hh
try {
  $d = dict[1.1 => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $e = dict[false => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $f = dict[true => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $g = dict[null => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $h = dict[array() => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $i = dict[new stdclass => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $j = false;
  $k = dict[$j =>1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
