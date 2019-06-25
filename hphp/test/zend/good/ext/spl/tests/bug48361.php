<?hh <<__EntryPoint>> function main(): void {
$info = new SplFileInfo(__FILE__);
var_dump($info->getRealPath());
var_dump($info->getPathInfo()->getRealPath());
echo "===DONE===\n";
}
