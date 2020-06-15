<?hh
<<__EntryPoint>> function main(): void {
$filename = __SystemLib\hphp_test_tmppath('SplFileInfo_getGroup_basic');
touch($filename);
$fileInfo = new SplFileInfo($filename);
$expected = filegroup($filename);
$actual = $fileInfo->getGroup();
var_dump($expected == $actual);

unlink($filename);
}
