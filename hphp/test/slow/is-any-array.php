<?hh // strict

class Foo { }

$foo = varray[
  varray[],
  darray[],
  array(),
  vec[],
  dict[],
  keyset[],
  3,
  "what",
  new Foo(),
];

function flib_is_any_array($x) {
  return is_array($x) || is_vec($x) || is_dict($x) || is_keyset($x);
}


foreach($foo as $val) {
  echo "-----\n";
  var_dump($val);
  var_dump(HH\is_any_array($val));
  var_dump(flib_is_any_array($val));
}
