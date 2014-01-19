<?php

//file
touch ('SplFileInfo_getInode_basic.txt');
$fileInfo = new SplFileInfo('SplFileInfo_getInode_basic.txt');
$result = shell_exec('ls -i SplFileInfo_getInode_basic.txt');
var_dump($fileInfo->getInode() == $result);

?>
<?php
unlink('SplFileInfo_getInode_basic.txt');
?>