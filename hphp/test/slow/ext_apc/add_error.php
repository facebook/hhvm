<?hh


<<__EntryPoint>>
function main_add_error() :mixed{
$entries = dict[];
$entries['key1'] = 'value1';
$entries['key2'] = 'value2';
$entries['key3'] = vec['value3a','value3b'];
$entries['key4'] = 4;

apc_store($entries);
var_dump(apc_add($entries)); // Error on all four, duplicates
}
