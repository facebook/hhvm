<?php
$fname = dirname(__FILE__)."/test53885.zip";
if(file_exists($fname)) unlink($fname);
touch($fname);
$nx=new ZipArchive();
$nx->open($fname);
$nx->locateName("a",ZIPARCHIVE::FL_UNCHANGED);
$nx->statName("a",ZIPARCHIVE::FL_UNCHANGED);
?>
==DONE==
<?php error_reporting(0); ?>
<?php
$fname = dirname(__FILE__)."/test53885.zip";
unlink($fname);
?>