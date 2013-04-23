<?php

include __DIR__.'/../../../test/sample_dir/fix_mtimes.inc';

$info = new SplFileInfo(__DIR__.'/../../sample_dir');
if (!$info->isFile()) {
  echo $info->getRealPath();
}
$info = new SplFileInfo(__DIR__.'/../../sample_dir/file');
var_dump($info->getbaseName());
var_dump($info->getbaseName('.cpp'));
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
