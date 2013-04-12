<?php

 $dir = new DirectoryIterator('test/sample_dir');while($dir->valid()) {  if(!$dir->isDot()) {    print $dir->current()."\n";  }  $dir->next();}