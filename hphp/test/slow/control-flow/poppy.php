<?hh


function poppy($arr) {
  $ret = $arr[1];
  $ret = $ret ?: varray[];
  return $ret;
}


<<__EntryPoint>>
function main_poppy() {
for ($i = 0; $i < 20; $i++) {
  var_dump(poppy(varray[null, varray[]]));
  var_dump(poppy(varray[null, varray[]]));
  var_dump(poppy(varray[null, varray[]]));
  var_dump(poppy(varray[null, varray[1,2]]));
}
}
