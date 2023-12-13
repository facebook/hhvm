<?hh

function foo($x) {
  var_dump(vec[1, $x]);
  var_dump(vec[1]);
  var_dump(dict['a' => $x]);
  var_dump(dict['a' => 1]);
  var_dump(vec[1, $x]);
  var_dump(dict['a' => $x]);
}
