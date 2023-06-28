<?hh


<<__EntryPoint>>
function main_double_start() :mixed{
ob_start();
ob_start();
die('print me');
}
