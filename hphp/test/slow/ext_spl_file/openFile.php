<?hh


<<__EntryPoint>>
function main_open_file() :mixed{
$info = new SplFileInfo(__DIR__.'/../../sample_dir/file');
var_dump($info->openFile()->fgets());
}
