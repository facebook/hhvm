<?hh


<<__EntryPoint>>
function main_to_dict() {
$s = shape(
  'x' => 4
);

var_dump(Shapes::toDict($s));
}
