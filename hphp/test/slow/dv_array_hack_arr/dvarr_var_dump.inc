<?hh

function foo($x) {
  var_dump(varray[1, $x]);
  var_dump(varray[1]);
  var_dump(darray['a' => $x]);
  var_dump(darray['a' => 1]);
  var_dump(vec[1, $x]);
  var_dump(dict['a' => $x]);
}
