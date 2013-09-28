<?php

function foo(&$a) {
}
foo($a[array()]);
foo($a[new StdClass]);
var_dump($a);
