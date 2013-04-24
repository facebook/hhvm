<?php

//file
touch ('test_file_ptfi');
chmod('test_file_ptfi', 0557);
$fileInfo = new SplFileInfo('test_file_ptfi');
var_dump($fileInfo->getPerms() == 0100557);

?><?php
unlink('test_file_ptfi');
?>