<?hh


<<__EntryPoint>>
function main_pretty_empty() :mixed{
var_dump(json_encode(vec[], JSON_PRETTY_PRINT));
var_dump(json_encode(new stdClass, JSON_PRETTY_PRINT));
}
