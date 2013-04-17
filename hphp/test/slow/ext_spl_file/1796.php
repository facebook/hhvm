<?php

chdir(__DIR__.'/../../..');

$info = new SplFileInfo('test/sample_dir');
var_dump($info->getRealPath());
var_dump($info->getPath());
var_dump($info->getPathName());
$info = new SplFileInfo('test/sample_dir/');
var_dump($info->getRealPath());
var_dump($info->getPath());
var_dump($info->getPathName());
$info = new SplFileInfo('test/sample_dir//../sample_dir');
var_dump($info->getRealPath());
var_dump($info->getPath());
var_dump($info->getPathName());
$p=realpath('test');
$info = new SplFileInfo($p.'/sample_dir/symlink');
var_dump($info->getLinkTarget());
var_dump($info->getRealPath());
var_dump($info->getPath());
var_dump($info->getPathName());
