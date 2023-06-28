<?hh


<<__EntryPoint>>
function main_1691() :mixed{
$fp = fopen(__DIR__.'/../../sample_dir/file', 'r');
var_dump(pclose($fp));
}
