<?hh

<<__EntryPoint>>
function main() :mixed{
    var_dump(array_diff(Map{"1" => 42}, dict[]));
    var_dump(array_diff_key(Map{"1" => 42}, dict[]));
}
