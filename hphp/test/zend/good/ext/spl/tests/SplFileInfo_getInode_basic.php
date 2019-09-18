<?hh
<<__EntryPoint>> function main(): void {
//file
touch ('SplFileInfo_getInode_basic.txt');
$fileInfo = new SplFileInfo('SplFileInfo_getInode_basic.txt');
$result = shell_exec('ls -i SplFileInfo_getInode_basic.txt');
var_dump($fileInfo->getInode() == $result);
error_reporting(0);
unlink('SplFileInfo_getInode_basic.txt');
}
