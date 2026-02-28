<?hh


<<__EntryPoint>>
function main_collection_literal_list_assign() :mixed{
list($x, $y) = Vector {'a', 'b'};
var_dump($x, $y);
}
