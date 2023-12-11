<?hh


function poppy($arr) :mixed{
  $ret = $arr[1];
  $ret = $ret ?: vec[];
  return $ret;
}


<<__EntryPoint>>
function main_poppy() :mixed{
for ($i = 0; $i < 20; $i++) {
  var_dump(poppy(vec[null, vec[]]));
  var_dump(poppy(vec[null, vec[]]));
  var_dump(poppy(vec[null, vec[]]));
  var_dump(poppy(vec[null, vec[1,2]]));
}
}
