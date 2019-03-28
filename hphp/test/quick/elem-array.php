<?hh


function go($a) {
  if ($a) $x = 5;

  $a[1] = array();
  $a[1]['hello'] = 5;
  if (isset($a[2]['hi'])) return true;
  return false;
}

$a = array();
var_dump(go($a));
