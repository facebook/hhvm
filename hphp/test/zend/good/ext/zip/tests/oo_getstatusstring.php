<?php

$dirname = dirname(__FILE__) . '/';
$arch = new ZipArchive;
$arch->open($dirname.'foo.zip',ZIPARCHIVE::CREATE);
var_dump($arch->getStatusString());
//delete an index that does not exist - trigger error
$arch->deleteIndex(2);
var_dump($arch->getStatusString());
$arch->close();

?>
<?php
unlink($dirname.'foo.zip');
?>