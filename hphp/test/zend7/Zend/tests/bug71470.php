<?php

$array = [1, 2, 3];
foreach ($array as &$v) {
    die("foo\n");
}

?>
