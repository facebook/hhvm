<?php

if ($argc > 100) {
 $f = 'var_dump';
 }
 else {
 $f = 'sscanf';
 }
$auth = "24\tLewis Carroll";
$n = $f($auth, "%d\t%s %s", $id, $first, $last);
echo "$id,$first,$last\n";
