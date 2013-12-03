<?php

//file
touch ('SplFileInfo_getPerms_basic.txt');
chmod('SplFileInfo_getPerms_basic.txt', 0557);
$fileInfo = new SplFileInfo('SplFileInfo_getPerms_basic.txt');
var_dump($fileInfo->getPerms() == 0100557);

?>
<?php
unlink('SplFileInfo_getPerms_basic.txt');
?>