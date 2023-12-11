<?hh


function main($arr, $a, $b) :mixed{
  return $arr[$a + $b];
}

<<__EntryPoint>>
function main_addo_member_key() :mixed{
var_dump(main(vec[1, 2, 3, 4, 5, 6], 2, 1));
}
