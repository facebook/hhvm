<?hh


<<__EntryPoint>>
function main_to_dict() :mixed{
$s = shape(
  'x' => 4
);

var_dump(Shapes::toDict($s));
}
