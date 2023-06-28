<?hh


<<__EntryPoint>>
function main_1796() :mixed{
chdir(__DIR__.'/../../..');

$info = new SplFileInfo('test/sample_dir');
var_dump($info->getRealPath());
var_dump($info->getPath());
var_dump($info->getPathname());
$info = new SplFileInfo('test/sample_dir/');
var_dump($info->getRealPath());
var_dump($info->getPath());
var_dump($info->getPathname());
$info = new SplFileInfo('test/sample_dir//../sample_dir');
var_dump($info->getRealPath());
var_dump($info->getPath());
var_dump($info->getPathname());
$p=realpath('test');
$info = new SplFileInfo($p.'/sample_dir/symlink');
var_dump($info->getLinkTarget());
var_dump($info->getRealPath());
var_dump($info->getPath());
var_dump($info->getPathname());
}
