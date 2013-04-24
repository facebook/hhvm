<?php
$info = new SplFileInfo(__FILE__);
var_dump($info->getRealPath());
var_dump($info->getPathInfo()->getRealPath());
?>
===DONE===