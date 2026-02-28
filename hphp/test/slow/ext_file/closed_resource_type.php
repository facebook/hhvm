<?hh


<<__EntryPoint>>
function main_closed_resource_type() :mixed{
$fp = fopen('php://memory', 'r');
fclose($fp);

var_dump(get_resource_type($fp));
}
