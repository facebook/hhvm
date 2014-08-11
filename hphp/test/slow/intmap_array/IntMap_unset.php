<?hh

function cow_unset($arr) {
  foreach ($arr as $key => $val) {
    if ($val) {
      unset($arr[$key]);
    } else {
      unset($arr['123']);
    }
  }
  return $arr;
}

function main() {
  $a = miarray();
  $a[1] = "moo";
  unset($a["1"]);

  $a = miarray();
  $a[1] = "moo";
  unset($a[1]);
  var_dump($a);

  $a = miarray();
  $a[1] = "moo";
  unset($a["foo"]);

  $a = miarray();
  $key = "1";
  $a[1] = "moo";
  unset($a[$key]);

  $a = miarray();
  $key = 1;
  $a[1] = "moo";
  unset($a[$key]);
  var_dump($a);

  $a = miarray();
  $key = "foo";
  $a[1] = "moo";
  unset($a[$key]);

  $a = miarray();
  $a[1] = true;
  $a[2] = false;
  $b = cow_unset($a);
  $a[] = "warning";
}

main();
