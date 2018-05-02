<?php

$stat = fb_lazy_stat(__FILE__);
var_dump($stat == stat(__FILE__));

$lstat = fb_lazy_lstat(__FILE__);
var_dump($lstat == lstat(__FILE__));

$path = fb_lazy_realpath(__FILE__);
var_dump($path == realpath(__FILE__));

$path = fb_lazy_readlink(__FILE__);
var_dump($path == readlink(__FILE__));
