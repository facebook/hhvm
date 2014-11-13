<?php

$file = tempnam('/tmp', 'lchown');
$link = tempnam('/tmp', 'lchown');
touch($file);
@symlink($file, $link);

var_dump(lchown($link, 'ihopenomachinehasthisuserthatwouldbebad'));
var_dump(chown($file, 'ihopenomachinehasthisuserthatwouldbebad'));
