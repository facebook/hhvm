<?php
$ext = new ReflectionExtension("standard");
$inis = $ext->getINIEntries();
var_dump($inis["user_agent"]);
?>
