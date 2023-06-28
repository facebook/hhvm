<?hh


<<__EntryPoint>>
function main_array_uintersect_vararg() :mixed{
  $array1 = darray["a" => "green", "b" => "brown", "c" => "blue", 0 => "red"];
  $array2 = darray["a" => "GREEN", "B" => "brown", 0 => "yellow", 1 => "red"];
  $array3 = darray["a" => "grEEN", 0 => "white"];

print_r(array_uintersect($array1, $array2, strcasecmp<>));
print_r(array_uintersect($array1, $array2, $array3, strcasecmp<>));
}
