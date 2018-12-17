<?hh

<<__EntryPoint>>
function main() {
  var_dump(array_change_key_case(Map{"1" => 42}));
  var_dump(array_change_key_case(dict["1" => 42]));
}
