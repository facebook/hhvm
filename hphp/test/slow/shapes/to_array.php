<?hh


<<__EntryPoint>>
function main_to_array() :mixed{
$s = shape(
  'x' => 4
);

var_dump(Shapes::toArray($s));
}
