<?hh


<<__EntryPoint>>
function main_array_search() :mixed{
$array = darray[
  0 => "blue",
  1 => "red",
  2 => "green",
  3 => "red"
];

var_dump(array_search("green", $array));
var_dump(array_search("red", $array));
}
