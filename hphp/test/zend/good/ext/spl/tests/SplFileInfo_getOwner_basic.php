<?hh
<<__EntryPoint>> function main(): void {
$filename = __SystemLib\hphp_test_tmppath('SplFileInfo_getOwner_basic');
touch($filename);
$fileInfo = new SplFileInfo($filename);
$expected = fileowner($filename);
$actual = $fileInfo->getOwner();
var_dump($expected == $actual);

unlink($filename);
}
