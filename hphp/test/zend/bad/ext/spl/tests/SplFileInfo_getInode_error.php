<?php

//file
$fileInfo = new SplFileInfo('not_existing');
var_dump($fileInfo->getInode());
?>
