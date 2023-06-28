<?hh


<<__EntryPoint>>
function main_1692() :mixed{
$fp = fopen('test/nonexist.txt', 'r');
var_dump(pclose($fp));
}
