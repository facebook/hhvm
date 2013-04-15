<?php

 $path = "test/sample_dir/";foreach (new RecursiveIteratorIterator(  new RecursiveDirectoryIterator($path,  RecursiveDirectoryIterator::KEY_AS_PATHNAME),  RecursiveIteratorIterator::CHILD_FIRST) as $file => $info) {  if ($info->isDir() && substr($file,-1)!='.') {    echo $file."\n";  }}