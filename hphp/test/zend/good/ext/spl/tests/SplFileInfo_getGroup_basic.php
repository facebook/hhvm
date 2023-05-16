<?hh
<<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'.'SplFileInfo_getGroup_basic';
touch($filename);
$fileInfo = new SplFileInfo($filename);
$expected = filegroup($filename);
$actual = $fileInfo->getGroup();
var_dump($expected == $actual);

unlink($filename);
}
