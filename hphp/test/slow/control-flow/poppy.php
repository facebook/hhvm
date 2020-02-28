<?hh


function poppy($arr) {
  $ret = $arr[1];
  $ret = $ret ?: array();
  return $ret;
}


<<__EntryPoint>>
function main_poppy() {
for ($i = 0; $i < 20; $i++) {
  var_dump(poppy(varray[null, array()]));
  var_dump(poppy(varray[null, array()]));
  var_dump(poppy(varray[null, array()]));
  var_dump(poppy(varray[null, varray[1,2]]));
}
}
