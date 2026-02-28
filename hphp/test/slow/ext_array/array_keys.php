<?hh

<<__EntryPoint>>
function main_array_keys() :mixed{
var_dump(array_keys(dict[0 => 100, "color" => "red"]));
var_dump(array_keys(
  dict["color" => vec["blue", "red", "green"],
        "size" => vec["small", "medium", "large"]]
));

$array = dict[
  "a" => null,
  "b" => 123,
  "c" => false,
];
var_dump(array_keys($array));
}
