<?php
class Foo extends SplFileObject
{
    public $bam = array();
}
$fileInfo = new SplFileInfo('php://temp');
$fileInfo->setFileClass('Foo');
$file = $fileInfo->openFile('r');

print var_dump($file->bam); // is null or UNKNOWN:0
?>
===DONE===