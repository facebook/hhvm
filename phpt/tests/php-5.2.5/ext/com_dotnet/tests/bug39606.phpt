--TEST--
COM: Loading typelib corrupts memory
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded("com_dotnet")) print "skip COM/.Net support not present"; ?>
--FILE--
<?php // $Id: bug39606.phpt,v 1.1.2.1 2006/12/09 10:52:09 rrichards Exp $
error_reporting(E_ALL);

$arEnv = array_change_key_case($_SERVER, CASE_UPPER);

$root = dirname($arEnv['COMSPEC']);
$typelib = $root.'\activeds.tlb';

var_dump(com_load_typelib($typelib));
var_dump(com_load_typelib($typelib));
?>
===DONE===
--EXPECT--
bool(true)
bool(true)
===DONE===