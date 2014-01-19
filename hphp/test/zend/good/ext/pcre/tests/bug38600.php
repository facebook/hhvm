<?php
$foo = 'bla bla bla';

var_dump(preg_match('/(?<!\w)(0x[\p{N}]+[lL]?|[\p{Nd}]+(e[\p{Nd}]*)?[lLdDfF]?)(?!\w)/', $foo, $m));
var_dump($m);

?>