<?hh

function test($a) :mixed{
  $ret = varray[];
  $s = varray[];
  foreach ($a as $v) {
    foreach ($v as $t) {
      $s = $s ?? varray[];
      $s[] = $t;
    }
    $s = $s ?? varray[];
    $ret[] = $s;
    unset($s);
  }
  return $ret;
}


<<__EntryPoint>>
function main_cgetquietl_uninit() :mixed{
for ($i = 0; $i < 50; $i++) {
  test(varray[
         varray[1,2,3],
         varray[],
         varray[],
         varray[4,5,6],
         varray[],
       ]);
}

var_dump(test(varray[
                varray[1,2,3],
                varray[],
                varray[],
                varray[4,5,6],
                varray[],
              ]));
}
