<?hh


function main($arr, $a, $b) {
  return $arr[$a + $b];
}

<<__EntryPoint>>
function main_addo_member_key() {
var_dump(main(varray[1, 2, 3, 4, 5, 6], 2, 1));
}
