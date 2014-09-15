<?hh

function a($x) {
  $z = $x ? array() : null;
  $z[] = 2;
  if ($z) {
    var_dump((bool)$z);
  }
  var_dump((bool)$z);
  return $z;
}

a(true);
a(false);
