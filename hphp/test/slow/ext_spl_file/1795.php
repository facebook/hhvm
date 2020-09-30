<?hh



<<__EntryPoint>> function main(): void {
$info = new SplFileInfo(__DIR__.'/../../sample_dir');
if (!$info->isFile()) {
  echo $info->getRealPath();
}
$info = new SplFileInfo(__DIR__.'/../../sample_dir/file');
var_dump($info->getBasename());
var_dump($info->getBasename('.cpp'));
$info->getCTime();
$info->getGroup();
$info->getInode();
$info->getMTime();
$info->getOwner();
$info->getPerms();
$info->getSize();
$info->getType();
$info->isDir();
$info->isFile();
$info->isLink();
$info->isReadable();
$info->isWritable();
}
