<?hh


function poppy($arr) {
  $ret = $arr[1];
  $ret = $ret ?: array();
  return $ret;
}

for ($i = 0; $i < 20; $i++) {
  var_dump(poppy(array(null, array())));
  var_dump(poppy(array(null, array())));
  var_dump(poppy(array(null, array())));
  var_dump(poppy(array(null, array(1,2))));
}
