<?hh
<<__EntryPoint>> function main(): void {
chdir(sys_get_temp_dir());
touch ('SplFileInfo_getInode_basic.txt');
$fileInfo = new SplFileInfo('SplFileInfo_getInode_basic.txt');
$result = shell_exec('ls -i SplFileInfo_getInode_basic.txt');
var_dump(HH\Lib\Legacy_FIXME\eq($fileInfo->getInode(), $result));

unlink('SplFileInfo_getInode_basic.txt');
}
