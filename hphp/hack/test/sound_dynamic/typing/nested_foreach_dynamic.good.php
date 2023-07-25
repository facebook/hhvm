<?hh

function f(dynamic $d) : void {
  $v1 = vec[];
  if ($d) { $v1 = $d; }
  $v1[] = vec[];
  $v1[0][] = $d;
  foreach ($v1 as $v2) {
    foreach ($v2 as $v3) {
    }
  }
}
