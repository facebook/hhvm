<?hh


<<__EntryPoint>>
function main_change_key_case() :mixed{
$input_array = dict["FirSt" => 1, "SecOnd" => 4];
var_dump(array_change_key_case($input_array, CASE_UPPER));
}
