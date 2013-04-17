<?php

chdir(__DIR__.'/../../..');
require 'test/sample_dir/fix_mtimes.inc';

$info = new SplFileInfo('test/sample_dir');
if (!$info->isFile()) {
  echo $info->getRealPath();
}
$info = new SplFileInfo('test/sample_dir/file');
var_dump($info->getbaseName());
var_dump($info->getbaseName('.cpp'));
var_dump($info->getCTime());
var_dump($info->getGroup());
var_dump($info->getInode());
var_dump($info->getMTime());
var_dump($info->getOwner());
var_dump($info->getPerms());
var_dump($info->getSize());
var_dump($info->getType());
var_dump($info->isDir());
var_dump($info->isFile());
var_dump($info->isLink());
var_dump($info->isReadable());
var_dump($info->isWritable());
