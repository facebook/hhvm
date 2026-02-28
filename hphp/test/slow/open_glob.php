<?hh


<<__EntryPoint>>
function main_open_glob() :mixed{
$res = fopen('glob://' . __DIR__ . '/../sample_dir/*', 'r');
var_dump($res);
}
