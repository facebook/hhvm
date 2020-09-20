<?hh


<<__EntryPoint>>
function main_open_glob() {
$res = fopen('glob://' . __DIR__ . '/../sample_dir/*', 'r');
var_dump($res);
}
