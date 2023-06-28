<?hh


<<__EntryPoint>>
function main_incomplete_class() :mixed{
$str = 'O:7:"invalid":0:{}';
$obj = unserialize($str);
var_dump($obj);
var_export($obj);
print_r($obj);
debug_zval_dump($obj);
}
