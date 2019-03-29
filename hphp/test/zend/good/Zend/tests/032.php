<?php

function test(&$var) { }
$arr = array();
test(&$arr[]);

print "ok!\n";
