<?hh


function main($a, &$x) {
  $x = $a[0];
  return empty($x) ? true : false;
}

echo main(array(array()), &$y)."\n";
