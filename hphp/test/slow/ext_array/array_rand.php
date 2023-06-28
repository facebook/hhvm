<?hh


<<__EntryPoint>>
function main_array_rand() :mixed{
$input = varray["Neo", "Morpheus", "Trinity", "Cypher", "Tank"];
$rand_keys = array_rand($input, 2);
var_dump(count($rand_keys));

foreach ($rand_keys as $k) {
  var_dump(array_key_exists($k, $input));
}
}
