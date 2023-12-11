<?hh


<<__EntryPoint>>
function main_array_uintersect_vararg() :mixed{
  $array1 = dict["a" => "green", "b" => "brown", "c" => "blue", 0 => "red"];
  $array2 = dict["a" => "GREEN", "B" => "brown", 0 => "yellow", 1 => "red"];
  $array3 = dict["a" => "grEEN", 0 => "white"];

print_r(array_uintersect($array1, $array2, strcasecmp<>));
print_r(array_uintersect($array1, $array2, $array3, strcasecmp<>));
}
