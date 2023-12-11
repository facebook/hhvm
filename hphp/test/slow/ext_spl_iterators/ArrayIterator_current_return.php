<?hh


<<__EntryPoint>>
function main_array_iterator_current_return() :mixed{
$arr = new ArrayIterator(vec[1]);

$arr->next();
var_dump($arr->current());
var_dump($arr->key());
}
