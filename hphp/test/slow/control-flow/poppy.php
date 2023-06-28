<?hh


function poppy($arr) :mixed{
  $ret = $arr[1];
  $ret = $ret ?: varray[];
  return $ret;
}


<<__EntryPoint>>
function main_poppy() :mixed{
for ($i = 0; $i < 20; $i++) {
  var_dump(poppy(varray[null, varray[]]));
  var_dump(poppy(varray[null, varray[]]));
  var_dump(poppy(varray[null, varray[]]));
  var_dump(poppy(varray[null, varray[1,2]]));
}
}
