<?hh

function test($a) {
  $ret = varray[];
  $s = varray[];
  foreach ($a as $v) {
    foreach ($v as $t) {
      $s[] = $t;
    }
    $s = $s ?? varray[];
    $ret[] = $s;
    unset($s);
  }
  return $ret;
}

for ($i = 0; $i < 50; $i++) {
  test(array(
         array(1,2,3),
         array(),
         array(),
         array(4,5,6),
         array(),
       ));
}

var_dump(test(array(
                array(1,2,3),
                array(),
                array(),
                array(4,5,6),
                array(),
              )));
