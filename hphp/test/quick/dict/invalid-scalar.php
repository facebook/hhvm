<?hh <<__EntryPoint>> function main(): void {
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
  $h = dict[dict[] => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $i = dict[new stdClass => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $j = false;
  $k = dict[$j =>1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $l = dict[vec[] => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $m = dict[dict[] => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $n = dict[keyset[] => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
try {
  $o = dict[Vector{1, 2, 3} => 1];
} catch (Exception $e) {
  var_dump("Got ".$e->getMessage());
}
}
