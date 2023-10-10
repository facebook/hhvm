<?hh

class Foo { }

function flib_is_any_array($x) :mixed{
  return is_array($x) || is_vec($x) || is_dict($x) || is_keyset($x);
}


<<__EntryPoint>>
function main_is_any_array() :mixed{
$foo = varray[
  varray[],
  darray[],
  varray[],
  vec[],
  dict[],
  keyset[],
  3,
  "what",
  new Foo(),
];


foreach($foo as $val) {
  echo "-----\n";
  var_dump($val);
  var_dump(HH\is_any_array($val));
  var_dump(flib_is_any_array($val));
}
}
