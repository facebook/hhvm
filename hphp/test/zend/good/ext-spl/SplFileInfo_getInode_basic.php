<?php

//file
touch ('test_file_ptfi');
$fileInfo = new SplFileInfo('test_file_ptfi');
$result = shell_exec('ls -i test_file_ptfi');
var_dump($fileInfo->getInode() == $result);

?><?php
unlink('test_file_ptfi');
?>