<?php

$file = '/etc/passwd'.chr(0).'asdf';

var_dump(gzopen($file, 'r'));
var_dump(gzfile($file));
var_dump(readgzfile($file));
