<?hh

<<__EntryPoint>>
function main_array_keys() :mixed{
var_dump(array_keys(darray[0 => 100, "color" => "red"]));
var_dump(array_keys(
  darray["color" => varray["blue", "red", "green"],
        "size" => varray["small", "medium", "large"]]
));

$array = darray[
  "a" => null,
  "b" => 123,
  "c" => false,
];
var_dump(array_keys($array));
}
