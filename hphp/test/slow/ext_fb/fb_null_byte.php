<?php

$file = '/etc/passwd'.chr(0).'asdf';

var_dump(fb_lazy_lstat($file));
var_dump(fb_lazy_realpath($file));
var_dump(fb_lazy_stat($file));
var_dump(fb_lazy_readlink($file));
