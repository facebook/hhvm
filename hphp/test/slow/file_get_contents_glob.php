<?hh


<<__EntryPoint>>
function main_file_get_contents_glob() :mixed{
$res = file_get_contents('glob://' . __DIR__ . '/../test/sample_dir/*');
var_dump($res);
}
