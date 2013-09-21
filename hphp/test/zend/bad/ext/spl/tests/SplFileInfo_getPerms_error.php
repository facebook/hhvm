<?php

//file
$fileInfo = new SplFileInfo('not_existing');
var_dump($fileInfo->getPerms() == 0100557);
?>
