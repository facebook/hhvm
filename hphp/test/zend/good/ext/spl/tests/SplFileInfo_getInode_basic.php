<?hh
<<__EntryPoint>> function main(): void {
chdir(__SystemLib\hphp_test_tmproot());
touch ('SplFileInfo_getInode_basic.txt');
$fileInfo = new SplFileInfo('SplFileInfo_getInode_basic.txt');
$result = shell_exec('ls -i SplFileInfo_getInode_basic.txt');
var_dump($fileInfo->getInode() == $result);

unlink('SplFileInfo_getInode_basic.txt');
}
