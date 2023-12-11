<?hh

function test($a) :mixed{
  $ret = vec[];
  $s = vec[];
  foreach ($a as $v) {
    foreach ($v as $t) {
      $s = $s ?? vec[];
      $s[] = $t;
    }
    $s = $s ?? vec[];
    $ret[] = $s;
    unset($s);
  }
  return $ret;
}


<<__EntryPoint>>
function main_cgetquietl_uninit() :mixed{
for ($i = 0; $i < 50; $i++) {
  test(vec[
         vec[1,2,3],
         vec[],
         vec[],
         vec[4,5,6],
         vec[],
       ]);
}

var_dump(test(vec[
                vec[1,2,3],
                vec[],
                vec[],
                vec[4,5,6],
                vec[],
              ]));
}
