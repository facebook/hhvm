<?hh


<<__EntryPoint>>
function main_528() {
  $input = darray["a" => "green",               0 => "red", "b" => "green", 1 => "blue", 2 => "red"];
$result = array_unique($input);
print_r($result);
}
