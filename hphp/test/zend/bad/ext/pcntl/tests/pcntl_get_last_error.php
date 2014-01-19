<?php 
var_dump(pcntl_get_last_error());
$pid = pcntl_wait($status);
var_dump($pid);
var_dump(pcntl_get_last_error() == PCNTL_ECHILD);
?>