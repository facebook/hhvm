<?hh


<<__EntryPoint>>
function main_popen_bad_args() :mixed{
$hurr = 'A';
$durr = '';

$res = popen($hurr, $durr);
var_dump($res);
}
