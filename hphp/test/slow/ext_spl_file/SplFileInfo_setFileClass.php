<?php

class MyFileObject extends SplFileObject {}

$info = new SplFileInfo(__FILE__);

$info->setFileClass('MyFileObject');
echo get_class($info->openFile()), "\n";

$info->setFileClass('SplFileObject');
echo get_class($info->openFile()), "\n";

try {
    $info->setFileClass('stdClass');
} catch (UnexpectedValueException $e) {
    echo $e->getMessage(), "\n";
}
