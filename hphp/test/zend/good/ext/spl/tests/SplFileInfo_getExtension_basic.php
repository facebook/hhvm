<?php
$file = md5('SplFileInfo::getExtension');
$exts = array('.txt', '.extension', '..', '.', '');
foreach ($exts as $ext) {
    touch($file . $ext);
    $info = new SplFileInfo($file . $ext);
    var_dump($info->getExtension(), pathinfo($file . $ext, PATHINFO_EXTENSION));
}
error_reporting(0);
$file = md5('SplFileInfo::getExtension');
$exts = array('.txt', '.extension', '..', '.', '');
foreach ($exts as $ext) {
    unlink($file . $ext);
}
